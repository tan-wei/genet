#include "../attr.hpp"
#include "attr.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void AttrWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Attr").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("value").ToLocalChecked(), value, setValue);
  Nan::SetAccessor(otl, Nan::New("type").ToLocalChecked(), type, setType);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  ctor->Set(Nan::New("create").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(AttrWrapper::create)->GetFunction());
  module->attribute.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->attribute.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Attr").ToLocalChecked(), ctor);
}

AttrWrapper::AttrWrapper(Attr *attr) : attr(attr), constProp(attr) {}

AttrWrapper::AttrWrapper(const Attr *attr) : attr(nullptr), constProp(attr) {}

AttrWrapper::AttrWrapper(const std::shared_ptr<Attr> &attr)
    : attr(attr.get()), constProp(attr.get()), shared(attr) {}

NAN_METHOD(AttrWrapper::create) {
  Token token = Token_null();
  auto id = info[0];
  if (id->IsUint32()) {
    token = id->Uint32Value();
  } else if (id->IsString()) {
    token = Token_get(*Nan::Utf8String(id));
  } else {
    Nan::ThrowTypeError("First argument must be a string or token-id");
    return;
  }
  info.GetReturnValue().Set(AttrWrapper::wrap(std::make_shared<Attr>(token)));
}

NAN_METHOD(AttrWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(AttrWrapper::id) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(attr->id())).ToLocalChecked());
  }
}

NAN_GETTER(AttrWrapper::range) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(static_cast<uint32_t>(attr->range().begin)));
    array->Set(1, Nan::New(static_cast<uint32_t>(attr->range().end)));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(AttrWrapper::setRange) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        attr->setRange(
            Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()});
      }
    }
  }
}

NAN_GETTER(AttrWrapper::value) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(Variant::getValue(attr->value()));
  }
}

NAN_SETTER(AttrWrapper::setValue) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    attr->setValue(Variant::getVariant(value));
  }
}

NAN_GETTER(AttrWrapper::type) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(attr->type())).ToLocalChecked());
  }
}

NAN_SETTER(AttrWrapper::setType) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    Token token = value->IsUint32() ? value->Uint32Value()
                                    : Token_get(*Nan::Utf8String(value));
    attr->setType(token);
  }
}

v8::Local<v8::Object> AttrWrapper::wrap(Attr *attr) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->attribute.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  AttrWrapper *wrapper = new AttrWrapper(attr);
  wrapper->Wrap(obj);
  return obj;
}

v8::Local<v8::Object> AttrWrapper::wrap(const Attr *attr) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->attribute.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  AttrWrapper *wrapper = new AttrWrapper(attr);
  wrapper->Wrap(obj);
  return obj;
}

v8::Local<v8::Object> AttrWrapper::wrap(const std::shared_ptr<Attr> &attr) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->attribute.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  AttrWrapper *wrapper = new AttrWrapper(attr);
  wrapper->Wrap(obj);
  return obj;
}

const Attr *AttrWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->attribute.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<AttrWrapper>(obj)) {
      return wrapper->constProp;
    }
  }
  return nullptr;
}
} // namespace plugkit
