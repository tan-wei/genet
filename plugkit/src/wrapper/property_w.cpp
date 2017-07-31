#include "property.hpp"
#include "../plugkit/property.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"

namespace plugkit {

void PropertyWrapper::init(v8::Isolate *isolate,
                           v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Property").ToLocalChecked());
  SetPrototypeMethod(tpl, "propertyFromId", propertyFromId);
  SetPrototypeMethod(tpl, "addProperty", addProperty);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id, setId);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("value").ToLocalChecked(), value, setValue);
  Nan::SetAccessor(otl, Nan::New("properties").ToLocalChecked(), properties);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->property.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->property.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Property").ToLocalChecked(), func);
}

PropertyWrapper::PropertyWrapper(Property *prop)
    : prop(prop), constProp(prop) {}

PropertyWrapper::PropertyWrapper(const Property *prop) : constProp(prop) {}

NAN_METHOD(PropertyWrapper::New) {
  if (!info[0]->IsObject()) {
    return;
  }
  auto options = info[0].As<v8::Object>();
  auto idValue = options->Get(Nan::New("id").ToLocalChecked());
  auto rangeValue = options->Get(Nan::New("range").ToLocalChecked());
  auto value = options->Get(Nan::New("value").ToLocalChecked());
  if (idValue->IsString()) {
    return;
  }
  auto prop = new Property(Token_get(*Nan::Utf8String(idValue)));
  if (rangeValue->IsArray()) {
    auto array = rangeValue.As<v8::Array>();
    if (array->Length() >= 2) {
      prop->setRange(
          Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()});
    }
  }
  prop->setValue(Variant::Private::getVariant(value));

  PropertyWrapper *obj = new PropertyWrapper(prop);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(PropertyWrapper::id) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(prop->id())).ToLocalChecked());
  }
}

NAN_SETTER(PropertyWrapper::setId) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setId(Token_get(*Nan::Utf8String(value)));
  }
}

NAN_GETTER(PropertyWrapper::range) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(prop->range().begin));
    array->Set(1, Nan::New(prop->range().end));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(PropertyWrapper::setRange) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
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

NAN_GETTER(PropertyWrapper::value) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Variant::Private::getValue(prop->value()));
  }
}

NAN_SETTER(PropertyWrapper::setValue) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setValue(Variant::Private::getVariant(value));
  }
}

NAN_GETTER(PropertyWrapper::properties) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &properties = prop->properties();
    auto array = v8::Array::New(isolate, properties.size());
    for (size_t i = 0; i < properties.size(); ++i) {
      array->Set(i, PropertyWrapper::wrap(properties[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(PropertyWrapper::propertyFromId) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    if (const auto &child =
            prop->propertyFromId(Token_get(*Nan::Utf8String(info[0])))) {
      info.GetReturnValue().Set(PropertyWrapper::wrap(child));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(PropertyWrapper::addProperty) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    if (info[0]->IsObject()) {
      if (const auto &child =
              PropertyWrapper::unwrap(info[0].As<v8::Object>())) {
        prop->addProperty(child);
      }
    }
  }
}

v8::Local<v8::Object> PropertyWrapper::wrap(const Property *prop) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->property.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  PropertyWrapper *wrapper = new PropertyWrapper(prop);
  wrapper->Wrap(obj);
  return obj;
}

const Property *PropertyWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->property.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<PropertyWrapper>(obj)) {
      return wrapper->constProp;
    }
  }
  return nullptr;
}
}
