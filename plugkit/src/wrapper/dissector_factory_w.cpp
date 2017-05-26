#include "dissector_factory.hpp"
#include "../plugkit/dissector.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"

namespace plugkit {
void DissectorFactoryWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("DissectorFactory").ToLocalChecked());

  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);

  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->dissectorFactory.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->dissectorFactory.ctor.Reset(isolate, ctor);
}

DissectorFactoryWrapper::DissectorFactoryWrapper(
    const DissectorFactoryConstPtr &factory)
    : factory(factory) {}

NAN_METHOD(DissectorFactoryWrapper::New) {
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Object>
DissectorFactoryWrapper::wrap(const DissectorFactoryConstPtr &factory) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto cons =
      v8::Local<v8::Function>::New(isolate, module->dissectorFactory.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  DissectorFactoryWrapper *wrapper = new DissectorFactoryWrapper(factory);
  wrapper->Wrap(obj);
  return obj;
}

DissectorFactoryConstPtr
DissectorFactoryWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->dissectorFactory.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<DissectorFactoryWrapper>(obj)) {
      return wrapper->factory;
    }
  }
  return DissectorFactoryConstPtr();
}

v8::Local<v8::Object>
DissectorFactory::wrap(const DissectorFactoryConstPtr &factory) {
  return DissectorFactoryWrapper::wrap(factory);
}
}
