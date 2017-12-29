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

AttributeWrapper::AttributeWrapper(Attr *attr) : attr(attr), constProp(attr) {}

AttributeWrapper::AttributeWrapper(const Attr *attr)
    : attr(nullptr), constProp(attr) {}

NAN_METHOD(AttributeWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(AttributeWrapper::id) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(attr->id())).ToLocalChecked());
  }
}

NAN_GETTER(AttributeWrapper::range) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(static_cast<uint32_t>(attr->range().begin)));
    array->Set(1, Nan::New(static_cast<uint32_t>(attr->range().end)));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(AttributeWrapper::setRange) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
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

NAN_GETTER(AttributeWrapper::value) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(Variant::getValue(attr->value()));
  }
}

NAN_SETTER(AttributeWrapper::setValue) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    attr->setValue(Variant::getVariant(value));
  }
}

NAN_GETTER(AttributeWrapper::type) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(attr->type())).ToLocalChecked());
  }
}

NAN_SETTER(AttributeWrapper::setType) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    Token token = value->IsUint32() ? value->Uint32Value()
                                    : Token_get(*Nan::Utf8String(value));
    attr->setType(token);
  }
}

NAN_GETTER(AttributeWrapper::error) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->constProp) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(attr->error())).ToLocalChecked());
  }
}

NAN_SETTER(AttributeWrapper::setError) {
  AttributeWrapper *wrapper =
      ObjectWrap::Unwrap<AttributeWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    Token token = value->IsUint32() ? value->Uint32Value()
                                    : Token_get(*Nan::Utf8String(value));
    attr->setError(token);
  }
}

v8::Local<v8::Object> AttributeWrapper::wrap(Attr *attr) {
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

v8::Local<v8::Object> AttributeWrapper::wrap(const Attr *attr) {
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

const Attr *AttributeWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
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
