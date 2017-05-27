#include "property.hpp"
#include "../plugkit/property.hpp"
#include "extended_slot.hpp"
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
  Nan::SetAccessor(otl, Nan::New("name").ToLocalChecked(), name, setName);
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id, setId);
  Nan::SetAccessor(otl, Nan::New("summary").ToLocalChecked(), summary,
                   setSummary);
  Nan::SetAccessor(otl, Nan::New("error").ToLocalChecked(), error, setError);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("value").ToLocalChecked(), value, setValue);
  Nan::SetAccessor(otl, Nan::New("properties").ToLocalChecked(), properties);

  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->property.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->property.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Property").ToLocalChecked(), func);
}

PropertyWrapper::PropertyWrapper(const std::shared_ptr<Property> &prop)
    : prop(prop), constProp(prop) {}

PropertyWrapper::PropertyWrapper(const std::shared_ptr<const Property> &prop)
    : constProp(prop) {}

NAN_METHOD(PropertyWrapper::New) {
  auto prop = std::make_shared<Property>();
  if (info[0]->IsString()) {
    prop->setId(*Nan::Utf8String(info[0]));
  }
  if (info[1]->IsString()) {
    prop->setName(*Nan::Utf8String(info[1]));
  }
  prop->setValue(Variant::Private::getVariant(info[2]));
  PropertyWrapper *obj = new PropertyWrapper(prop);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(PropertyWrapper::name) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Nan::New(prop->name()).ToLocalChecked());
  }
}

NAN_SETTER(PropertyWrapper::setName) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setName(*Nan::Utf8String(value));
  }
}

NAN_GETTER(PropertyWrapper::id) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Nan::New(prop->id()).ToLocalChecked());
  }
}

NAN_SETTER(PropertyWrapper::setId) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setId(*Nan::Utf8String(value));
  }
}

NAN_GETTER(PropertyWrapper::range) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(prop->range().first));
    array->Set(1, Nan::New(prop->range().second));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(PropertyWrapper::setRange) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        prop->setRange(std::make_pair(array->Get(0)->Uint32Value(),
                                      array->Get(1)->Uint32Value()));
      }
    }
  }
}

NAN_GETTER(PropertyWrapper::summary) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Nan::New(prop->summary()).ToLocalChecked());
  }
}

NAN_SETTER(PropertyWrapper::setSummary) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setSummary(*Nan::Utf8String(value));
  }
}

NAN_GETTER(PropertyWrapper::error) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->constProp) {
    info.GetReturnValue().Set(Nan::New(prop->error()).ToLocalChecked());
  }
}

NAN_SETTER(PropertyWrapper::setError) {
  PropertyWrapper *wrapper = ObjectWrap::Unwrap<PropertyWrapper>(info.Holder());
  if (auto prop = wrapper->prop) {
    prop->setError(*Nan::Utf8String(value));
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
    if (const auto &child = prop->propertyFromId(*Nan::Utf8String(info[0]))) {
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

v8::Local<v8::Object>
PropertyWrapper::wrap(const std::shared_ptr<const Property> &prop) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto cons = v8::Local<v8::Function>::New(isolate, module->property.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  PropertyWrapper *wrapper = new PropertyWrapper(prop);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<const Property>
PropertyWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->property.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<PropertyWrapper>(obj)) {
      return wrapper->constProp;
    }
  }
  return std::shared_ptr<Property>();
}
}
