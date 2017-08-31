#include "attribute.hpp"
#include "../attribute.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void AttributeWrapper::init(v8::Isolate *isolate,
                            v8::Local<v8::Object> exports) {
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
  module->attribute.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->attribute.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Attr").ToLocalChecked(), func);
}

AttributeWrapper::AttributeWrapper(Attr *prop) : prop(prop), constProp(prop) {}

AttributeWrapper::AttributeWrapper(const Attr *prop) : constProp(prop) {}

NAN_METHOD(AttributeWrapper::New) {
  if (!info[0]->IsObject()) {
    return;
  }
  auto options = info[0].As<v8::Object>();
  auto idValue = options->Get(Nan::New("id").ToLocalChecked());
  auto rangeValue = options->Get(Nan::New("range").ToLocalChecked());
  auto typeValue = options->Get(Nan::New("type").ToLocalChecked());
  auto value = options->Get(Nan::New("value").ToLocalChecked());
  if (idValue->IsNumber()) {
    return;
  }
  auto prop = new Attr(Token_get(*Nan::Utf8String(idValue)));
  if (rangeValue->IsArray()) {
    auto array = rangeValue.As<v8::Array>();
    if (array->Length() >= 2) {
      prop->setRange(
          Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()});
    }
  }
  if (typeValue->IsString()) {
    prop->setType(Token_get(*Nan::Utf8String(typeValue)));
  }
  prop->setValue(Variant::getVariant(value));

  AttributeWrapper *obj = new AttributeWrapper(prop);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(AttributeWrapper::id) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(prop->id())).ToLocalChecked());
  }
}

NAN_GETTER(AttributeWrapper::range) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(static_cast<uint32_t>(prop->range().begin)));
    array->Set(1, Nan::New(static_cast<uint32_t>(prop->range().end)));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(AttributeWrapper::setRange) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        prop->setRange(
            Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()});
      }
    }
  }
}

NAN_GETTER(AttributeWrapper::value) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Variant::getValue(prop->value()));
  }
}

NAN_SETTER(AttributeWrapper::setValue) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setValue(Variant::getVariant(value));
  }
}

NAN_GETTER(AttributeWrapper::type) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(prop->type())).ToLocalChecked());
  }
}

NAN_SETTER(AttributeWrapper::setType) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setType(Token_get(*Nan::Utf8String(value)));
  }
}

v8::Local<v8::Object> AttributeWrapper::wrap(const Attr *prop) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->attribute.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  AttributeWrapper *wrapper = new AttributeWrapper(prop);
  wrapper->Wrap(obj);
  return obj;
}

const Attr *AttributeWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->attribute.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<AttributeWrapper>(obj)) {
      return wrapper->constProp;
    }
  }
  return nullptr;
}
}
