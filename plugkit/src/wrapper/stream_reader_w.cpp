#include "stream_reader.hpp"
#include "../../include/plugkit/stream_reader.h"

namespace plugkit {

StreamReaderWrapper::StreamReaderWrapper() : reader(StreamReader_create()) {}

StreamReaderWrapper::~StreamReaderWrapper() { StreamReader_destroy(reader); }

void StreamReaderWrapper::init(v8::Isolate *isolate,
                               v8::Local<v8::Object> exports) {
  auto obj = Nan::New<v8::Object>();
  Nan::Set(exports, Nan::New("StreamReader").ToLocalChecked(), obj);
}

NAN_METHOD(StreamReaderWrapper::New) {
  StreamReaderWrapper *obj = new StreamReaderWrapper();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}
}
