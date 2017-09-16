#include "reader.hpp"
#include "payload.hpp"
#include "variant.hpp"

namespace plugkit {

ReaderWrapper::ReaderWrapper() {}

void ReaderWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Reader").ToLocalChecked());
  SetPrototypeMethod(tpl, "getUint8", getUint8);
  SetPrototypeMethod(tpl, "getInt8", getInt8);
  SetPrototypeMethod(tpl, "getUint16", getUint16);
  SetPrototypeMethod(tpl, "getUint32", getUint32);
  SetPrototypeMethod(tpl, "getInt16", getInt16);
  SetPrototypeMethod(tpl, "getInt32", getInt32);
  SetPrototypeMethod(tpl, "getFloat32", getFloat32);
  SetPrototypeMethod(tpl, "getFloat64", getFloat64);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  // Nan::SetAccessor(otl, Nan::New("data").ToLocalChecked(), data, setData);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Reader").ToLocalChecked(), func);
}

NAN_METHOD(ReaderWrapper::New) {
  ReaderWrapper *obj = new ReaderWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(ReaderWrapper::getUint8) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getUint8(reader));
  }
}

NAN_METHOD(ReaderWrapper::getInt8) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getInt8(reader));
  }
}

NAN_METHOD(ReaderWrapper::getUint16) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(
        Reader_getUint16(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getUint32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(
        Reader_getUint32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getInt16) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getInt16(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getInt32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(Reader_getInt32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getFloat32) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(
        Reader_getFloat32(reader, info[0]->BooleanValue()));
  }
}

NAN_METHOD(ReaderWrapper::getFloat64) {
  ReaderWrapper *wrapper = ObjectWrap::Unwrap<ReaderWrapper>(info.Holder());
  if (Reader *reader = &wrapper->reader) {
    info.GetReturnValue().Set(
        Reader_getFloat64(reader, info[0]->BooleanValue()));
  }
}
}
