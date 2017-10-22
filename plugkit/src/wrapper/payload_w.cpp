#include "../payload.hpp"
#include "attr.hpp"
#include "payload.hpp"
#include "plugkit_module.hpp"
#include "variant.hpp"

namespace plugkit {

void PayloadWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Payload").ToLocalChecked());
  SetPrototypeMethod(tpl, "addSlice", addSlice);
  SetPrototypeMethod(tpl, "attr", attr);
  SetPrototypeMethod(tpl, "addAttr", addAttr);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("slices").ToLocalChecked(), slices);
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);
  Nan::SetAccessor(otl, Nan::New("attrs").ToLocalChecked(), attrs);
  Nan::SetAccessor(otl, Nan::New("type").ToLocalChecked(), type, setType);
  Nan::SetIndexedPropertyHandler(otl, indexGetter);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->payload.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->payload.ctor.Reset(isolate, ctor);
}

PayloadWrapper::PayloadWrapper(Payload *payload)
    : payload(payload), constPayload(payload) {}

PayloadWrapper::PayloadWrapper(const Payload *payload)
    : payload(nullptr), constPayload(payload) {}

NAN_METHOD(PayloadWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_METHOD(PayloadWrapper::addSlice) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->payload) {
    if (info[0]->IsArrayBufferView()) {
      auto view = info[0].As<v8::ArrayBufferView>();
      if (view->Buffer()->IsExternal()) {
        payload->addSlice(Variant::getSlice(view));
        return;
      }
    }
    Nan::ThrowTypeError(
        "First argument must be an externalized ArrayBufferView");
  }
}

NAN_GETTER(PayloadWrapper::slices) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &slices = payload->slices();
    auto array = v8::Array::New(isolate, slices.size());
    for (size_t i = 0; i < slices.size(); ++i) {
      array->Set(i, Variant::getNodeBuffer(slices[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(PayloadWrapper::length) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    info.GetReturnValue().Set(static_cast<uint32_t>(payload->length()));
  }
}

NAN_GETTER(PayloadWrapper::attrs) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &attrs = payload->attrs();
    auto array = v8::Array::New(isolate, attrs.size());
    for (size_t i = 0; i < attrs.size(); ++i) {
      array->Set(i, AttributeWrapper::wrap(attrs[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(PayloadWrapper::attr) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    Token token = Token_null();
    if (info[0]->IsNumber()) {
      token = info[0]->NumberValue();
    } else if (info[0]->IsString()) {
      token = Token_get(*Nan::Utf8String(info[0]));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }

    if (const auto &child = payload->attr(token)) {
      info.GetReturnValue().Set(AttributeWrapper::wrap(child));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(PayloadWrapper::addAttr) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->payload) {
    Token token = Token_null();
    if (info[0]->IsNumber()) {
      token = info[0]->NumberValue();
    } else if (info[0]->IsString()) {
      token = Token_get(*Nan::Utf8String(info[0]));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }

    info.GetReturnValue().Set(
        AttributeWrapper::wrap(Payload_addAttr(payload, token)));
  }
}

NAN_GETTER(PayloadWrapper::type) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(payload->type())).ToLocalChecked());
  }
}

NAN_SETTER(PayloadWrapper::setType) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->payload) {
    Token token = value->IsNumber() ? value->NumberValue()
                                    : Token_get(*Nan::Utf8String(value));
    payload->setType(token);
  }
}

NAN_INDEX_GETTER(PayloadWrapper::indexGetter) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    if (index < payload->length()) {
      uint32_t offset = 0;
      for (const Slice &slice : payload->slices()) {
        size_t len = Slice_length(slice);
        if (offset + len > index) {
          info.GetReturnValue().Set(
              static_cast<unsigned char>(slice.begin[index - offset]));
          return;
        }
        offset += len;
      }
    }
  }
}

v8::Local<v8::Object> PayloadWrapper::wrap(Payload *payload) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->payload.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  PayloadWrapper *wrapper = new PayloadWrapper(payload);
  wrapper->Wrap(obj);
  return obj;
}

v8::Local<v8::Object> PayloadWrapper::wrap(const Payload *payload) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->payload.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  PayloadWrapper *wrapper = new PayloadWrapper(payload);
  wrapper->Wrap(obj);
  return obj;
}

const Payload *PayloadWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->payload.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<PayloadWrapper>(obj)) {
      return wrapper->constPayload;
    }
  }
  return nullptr;
}
} // namespace plugkit
