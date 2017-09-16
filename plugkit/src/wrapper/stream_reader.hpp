#ifndef PLUGKIT_STREAM_READER_WRAPPER_H
#define PLUGKIT_STREAM_READER_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct StreamReader;

class StreamReaderWrapper final : public Nan::ObjectWrap {
public:
  ~StreamReaderWrapper();
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static NAN_METHOD(New);
  static NAN_GETTER(length);
  static NAN_METHOD(addPayload);
  static NAN_METHOD(addSlice);
  static NAN_METHOD(search);

private:
  StreamReaderWrapper();
  StreamReaderWrapper(const StreamReaderWrapper &) = delete;
  StreamReaderWrapper &operator=(const StreamReaderWrapper &) = delete;

private:
  StreamReader *reader;
};
}

#endif
