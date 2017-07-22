#include "slice.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void SliceWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Slice").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->slice.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->slice.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Slice").ToLocalChecked(), func);
}

SliceWrapper::SliceWrapper(const Slice &slice) : slice(slice) {}

NAN_METHOD(SliceWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(SliceWrapper::length) {
  SliceWrapper *wrapper = ObjectWrap::Unwrap<SliceWrapper>(info.Holder());
  info.GetReturnValue().Set(static_cast<uint32_t>(wrapper->slice.length()));
}

v8::Local<v8::Object> SliceWrapper::wrap(const Slice &slice) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->slice.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  SliceWrapper *wrapper = new SliceWrapper(slice);
  wrapper->Wrap(obj);
  return obj;
}

Slice SliceWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->slice.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<SliceWrapper>(obj)) {
      return wrapper->slice;
    }
  }
  return Slice();
}
}
