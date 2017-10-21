#include "../attr.hpp"
#include "attr.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void AttributeWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Attr").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("value").ToLocalChecked(), value, setValue);
  Nan::SetAccessor(otl, Nan::New("type").ToLocalChecked(), type, setType);
  Nan::SetAccessor(otl, Nan::New("error").ToLocalChecked(), error, setError);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->attribute.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->attribute.ctor.Reset(isolate, ctor);
}

AttributeWrapper::AttributeWrapper(Attr *prop) : prop(prop), constProp(prop) {}

AttributeWrapper::AttributeWrapper(const Attr *prop)
    : prop(nullptr), constProp(prop) {}

NAN_METHOD(AttributeWrapper::New) { info.GetReturnValue().Set(info.This()); }

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
    Token token = value->IsNumber() ? value->NumberValue()
                                    : Token_get(*Nan::Utf8String(value));
    prop->setType(token);
  }
}

NAN_GETTER(AttributeWrapper::error) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(prop->error())).ToLocalChecked());
  }
}

NAN_SETTER(AttributeWrapper::setError) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    Token token = value->IsNumber() ? value->NumberValue()
                                    : Token_get(*Nan::Utf8String(value));
    prop->setError(token);
  }
}

v8::Local<v8::Object> AttributeWrapper::wrap(Attr *prop) {
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
} // namespace plugkit
