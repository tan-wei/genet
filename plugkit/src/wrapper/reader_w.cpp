#include "reader.hpp"
#include "payload.hpp"
#include "variant.hpp"

namespace plugkit {

ReaderWrapper::ReaderWrapper() {}

void ReaderWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Reader").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("data").ToLocalChecked(), data, setData);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Reader").ToLocalChecked(), func);
}

NAN_METHOD(ReaderWrapper::New) {
  ReaderWrapper *obj = new ReaderWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}
}
