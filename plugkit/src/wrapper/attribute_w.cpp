#include "attribute.hpp"
#include "../plugkit/attribute.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"

namespace plugkit {

void AttributeWrapper::init(v8::Isolate *isolate,
                            v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Attribute").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id, setId);
  Nan::SetAccessor(otl, Nan::New("value").ToLocalChecked(), value, setValue);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->attribute.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->attribute.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Attribute").ToLocalChecked(), func);
}

AttributeWrapper::AttributeWrapper(const std::shared_ptr<Attribute> &attr)
    : attr(attr), weakAttr(attr) {}

AttributeWrapper::AttributeWrapper(const std::weak_ptr<Attribute> &attr)
    : weakAttr(attr) {}

NAN_METHOD(AttributeWrapper::New) {
  if (!info[0]->IsObject()) {
    return;
  }
  auto options = info[0].As<v8::Object>();
  auto idValue = options->Get(Nan::New("id").ToLocalChecked());
  auto value = options->Get(Nan::New("value").ToLocalChecked());
  if (idValue->IsString()) {
    return;
  }
  auto attr = std::make_shared<Attribute>(miniid(*Nan::Utf8String(idValue)));
  attr->setValue(Variant::Private::getVariant(value));

  AttributeWrapper *obj = new AttributeWrapper(attr);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(AttributeWrapper::id) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->weakAttr.lock()) {
    info.GetReturnValue().Set(Nan::New(attr->id().str()).ToLocalChecked());
  }
}

NAN_SETTER(AttributeWrapper::setId) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    attr->setId(miniid(*Nan::Utf8String(value)));
  }
}

NAN_GETTER(AttributeWrapper::value) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->weakAttr.lock()) {
    info.GetReturnValue().Set(Variant::Private::getValue(attr->value()));
  }
}

NAN_SETTER(AttributeWrapper::setValue) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    attr->setValue(Variant::Private::getVariant(value));
  }
}

v8::Local<v8::Object>
AttributeWrapper::wrap(const std::shared_ptr<Attribute> &attr) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->attribute.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  AttributeWrapper *wrapper = new AttributeWrapper(attr);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<Attribute> AttributeWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->attribute.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<AttributeWrapper>(obj)) {
      return wrapper->attr;
    }
  }
  return nullptr;
}

std::shared_ptr<const Attribute>
AttributeWrapper::unwrapConst(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->attribute.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<AttributeWrapper>(obj)) {
      return wrapper->weakAttr.lock();
    }
  }
  return nullptr;
}
}
