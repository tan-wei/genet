#include "listener_factory.hpp"
#include "../plugkit/listener.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"

namespace plugkit {
void ListenerFactoryWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("ListenerFactory").ToLocalChecked());

  PlugkitModule *module = PlugkitModule::get(isolate);

  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->listenerFactory.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->listenerFactory.ctor.Reset(isolate, ctor);
}

ListenerFactoryWrapper::ListenerFactoryWrapper(
    const ListenerFactoryConstPtr &factory)
    : factory(factory) {}

NAN_METHOD(ListenerFactoryWrapper::New) {
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Object>
ListenerFactoryWrapper::wrap(const ListenerFactoryConstPtr &factory) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons =
      v8::Local<v8::Function>::New(isolate, module->listenerFactory.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ListenerFactoryWrapper *wrapper = new ListenerFactoryWrapper(factory);
  wrapper->Wrap(obj);
  return obj;
}

ListenerFactoryConstPtr
ListenerFactoryWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->listenerFactory.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ListenerFactoryWrapper>(obj)) {
      return wrapper->factory;
    }
  }
  return ListenerFactoryConstPtr();
}

v8::Local<v8::Object>
ListenerFactory::wrap(const ListenerFactoryConstPtr &factory) {
  return ListenerFactoryWrapper::wrap(factory);
}
}
