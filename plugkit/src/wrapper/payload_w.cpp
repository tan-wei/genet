#include "payload.hpp"
#include "../payload.hpp"
#include "variant.hpp"
#include "attribute.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void PayloadWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Payload").ToLocalChecked());
  SetPrototypeMethod(tpl, "attr", attr);
  SetPrototypeMethod(tpl, "addAttr", addAttr);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("slices").ToLocalChecked(), slices);
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);
  Nan::SetAccessor(otl, Nan::New("properties").ToLocalChecked(), properties);
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
    : constPayload(payload) {}

NAN_METHOD(PayloadWrapper::New) { info.GetReturnValue().Set(info.This()); }

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

NAN_GETTER(PayloadWrapper::properties) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &properties = payload->properties();
    auto array = v8::Array::New(isolate, properties.size());
    for (size_t i = 0; i < properties.size(); ++i) {
      array->Set(i, AttributeWrapper::wrap(properties[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(PayloadWrapper::attr) {
  PayloadWrapper *wrapper = ObjectWrap::Unwrap<PayloadWrapper>(info.Holder());
  if (auto payload = wrapper->constPayload) {
    Token token = info[0]->IsNumber() ? info[0]->NumberValue()
                                      : Token_get(*Nan::Utf8String(info[0]));
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
    if (info[0]->IsObject()) {
      if (const auto &child =
              AttributeWrapper::unwrap(info[0].As<v8::Object>())) {
        payload->addAttr(child);
      }
    }
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
          info.GetReturnValue().Set(slice.begin[index - offset]);
          return;
        }
        offset += len;
      }
    }
  }
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

const Payload *PayloadWrapper::unwrap(v8::Local<v8::Object> obj) {
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
}
