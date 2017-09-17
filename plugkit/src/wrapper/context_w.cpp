#include "context.hpp"
#include "context.h"
#include "plugkit_module.hpp"
#include "variant.hpp"
#include <vector>

namespace plugkit {

void ContextWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Context").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("options").ToLocalChecked(), options);

  PlugkitModule *module = PlugkitModule::get(isolate);
  module->context.ctor.Reset(isolate, Nan::GetFunction(tpl).ToLocalChecked());
}

ContextWrapper::ContextWrapper(Context *ctx) : ctx(ctx) {}

NAN_METHOD(ContextWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(ContextWrapper::options) {
  ContextWrapper *wrapper = ObjectWrap::Unwrap<ContextWrapper>(info.Holder());
  if (Context *ctx = wrapper->ctx) {
    info.GetReturnValue().Set(Variant::getValue(Context_options(ctx)));
  }
}

v8::Local<v8::Object> ContextWrapper::wrap(Context *ctx) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->context.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ContextWrapper *wrapper = new ContextWrapper(ctx);
  wrapper->Wrap(obj);
  return obj;
}

Context *ContextWrapper::unwrap(v8::Local<v8::Object> obj) {
  if (auto wrapper = ObjectWrap::Unwrap<ContextWrapper>(obj)) {
    return wrapper->ctx;
  }
  return nullptr;
}
}
