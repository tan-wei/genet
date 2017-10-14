#include "plugkit_module.hpp"
#include "worker.hpp"

namespace plugkit {

WorkerWrapper::WorkerWrapper() {}

void WorkerWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Worker").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();

  v8::Local<v8::Function> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Worker").ToLocalChecked(), func);

  PlugkitModule *module = PlugkitModule::get(isolate);
  module->worker.proto.Reset(isolate,
                             func->Get(Nan::New("prototype").ToLocalChecked()));
  module->worker.temp.Reset(isolate, tpl);
}

bool WorkerWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto inst = obj->FindInstanceInPrototypeChain(
      v8::Local<v8::FunctionTemplate>::New(isolate, module->worker.temp));
  if (auto wrapper = ObjectWrap::Unwrap<WorkerWrapper>(inst)) {
    return true;
  }
  return false;
}

NAN_METHOD(WorkerWrapper::New) {
  WorkerWrapper *obj = new WorkerWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

} // namespace plugkit
