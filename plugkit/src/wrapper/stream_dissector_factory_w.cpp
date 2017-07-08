#include "stream_dissector_factory.hpp"
#include "../plugkit/stream_dissector.hpp"
#include "plugkit_module.hpp"

namespace plugkit {
void StreamDissectorFactoryWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("StreamDissectorFactory").ToLocalChecked());

  PlugkitModule *module = PlugkitModule::get(isolate);

  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->streamDissectorFactory.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->streamDissectorFactory.ctor.Reset(isolate, ctor);
}

StreamDissectorFactoryWrapper::StreamDissectorFactoryWrapper(
    const StreamDissectorFactoryConstPtr &factory)
    : factory(factory) {}

NAN_METHOD(StreamDissectorFactoryWrapper::New) {
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Object> StreamDissectorFactoryWrapper::wrap(
    const StreamDissectorFactoryConstPtr &factory) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate,
                                           module->streamDissectorFactory.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  StreamDissectorFactoryWrapper *wrapper =
      new StreamDissectorFactoryWrapper(factory);
  wrapper->Wrap(obj);
  return obj;
}

StreamDissectorFactoryConstPtr
StreamDissectorFactoryWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate,
                                module->streamDissectorFactory.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<StreamDissectorFactoryWrapper>(obj)) {
      return wrapper->factory;
    }
  }
  return StreamDissectorFactoryConstPtr();
}

v8::Local<v8::Object>
StreamDissectorFactory::wrap(const StreamDissectorFactoryConstPtr &factory) {
  return StreamDissectorFactoryWrapper::wrap(factory);
}
}
