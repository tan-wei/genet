#include "stream_reader.hpp"
#include "payload.hpp"
#include "variant.hpp"
#include "../../include/plugkit/stream_reader.h"

namespace plugkit {

StreamReaderWrapper::StreamReaderWrapper() : reader(StreamReader_create()) {}

StreamReaderWrapper::~StreamReaderWrapper() { StreamReader_destroy(reader); }

void StreamReaderWrapper::init(v8::Isolate *isolate,
                               v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("StreamReader").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Attr").ToLocalChecked(), func);
}

NAN_METHOD(StreamReaderWrapper::New) {
  StreamReaderWrapper *obj = new StreamReaderWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(StreamReaderWrapper::length) {
  StreamReaderWrapper *wrapper =
      ObjectWrap::Unwrap<StreamReaderWrapper>(info.Holder());
  if (auto reader = wrapper->reader) {
    info.GetReturnValue().Set(
        static_cast<uint32_t>(StreamReader_length(reader)));
  }
}

NAN_METHOD(StreamReaderWrapper::addPayload) {
  StreamReaderWrapper *wrapper =
      ObjectWrap::Unwrap<StreamReaderWrapper>(info.Holder());
  if (auto reader = wrapper->reader) {
    if (const Payload *payload =
            PayloadWrapper::unwrap(info[0].As<v8::Object>())) {
      StreamReader_addPayload(reader, payload);
    }
  }
}

NAN_METHOD(StreamReaderWrapper::addSlice) {
  StreamReaderWrapper *wrapper =
      ObjectWrap::Unwrap<StreamReaderWrapper>(info.Holder());
  if (auto reader = wrapper->reader) {
    if (info[0]->IsArrayBufferView()) {
      auto view = info[0].As<v8::ArrayBufferView>();
      if (view->Buffer()->IsExternal()) {
        StreamReader_addSlice(reader, Variant::getSlice(view));
      }
    }
  }
}
}
