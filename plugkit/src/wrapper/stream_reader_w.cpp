#include "payload.hpp"
#include "stream_reader.h"
#include "stream_reader.hpp"
#include "variant.hpp"

namespace plugkit {

StreamReaderWrapper::StreamReaderWrapper() : reader(StreamReader_create()) {}

StreamReaderWrapper::~StreamReaderWrapper() { StreamReader_destroy(reader); }

void StreamReaderWrapper::init(v8::Isolate *isolate,
                               v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("StreamReader").ToLocalChecked());
  SetPrototypeMethod(tpl, "search", search);
  SetPrototypeMethod(tpl, "read", read);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("StreamReader").ToLocalChecked(), func);
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

NAN_METHOD(StreamReaderWrapper::search) {
  StreamReaderWrapper *wrapper =
      ObjectWrap::Unwrap<StreamReaderWrapper>(info.Holder());
  if (auto reader = wrapper->reader) {
    if (info[0]->IsArrayBufferView()) {
      auto view = info[0].As<v8::ArrayBufferView>();
      size_t offset = 0;
      if (info[1]->IsNumber()) {
        offset = info[1]->NumberValue();
      }
      const char *data =
          static_cast<const char *>(view->Buffer()->GetContents().Data()) +
          view->ByteOffset();
      info.GetReturnValue().Set(static_cast<uint32_t>(
          StreamReader_search(reader, data, view->ByteLength(), offset)));
    }
  }
}

NAN_METHOD(StreamReaderWrapper::read) {
  StreamReaderWrapper *wrapper =
      ObjectWrap::Unwrap<StreamReaderWrapper>(info.Holder());
  if (auto reader = wrapper->reader) {
    size_t size = 0;
    size_t offset = 0;
    if (info[0]->IsNumber()) {
      size = info[0]->NumberValue();
    }
    if (info[1]->IsNumber()) {
      offset = info[1]->NumberValue();
    }
    auto buf = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), size);
    char *data = static_cast<char *>(buf->GetContents().Data());
    Slice slice = StreamReader_read(reader, data, size, offset);
    size_t sliceLen = Slice_length(slice);
    if (Slice_length(slice) == 0) {
      info.GetReturnValue().Set(v8::Uint8Array::New(buf, 0, 0));
    } else if (slice.begin == data) {
      info.GetReturnValue().Set(v8::Uint8Array::New(buf, 0, sliceLen));
    }
    info.GetReturnValue().Set(v8::Uint8Array::New(
        v8::ArrayBuffer::New(v8::Isolate::GetCurrent(),
                             const_cast<char *>(slice.begin), sliceLen),
        0, sliceLen));
  }
}
} // namespace plugkit
