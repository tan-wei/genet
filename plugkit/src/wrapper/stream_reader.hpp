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
  NAN_METHOD(New);

private:
  StreamReaderWrapper();
  StreamReaderWrapper(const StreamReaderWrapper &) = delete;
  StreamReaderWrapper &operator=(const StreamReaderWrapper &) = delete;

private:
  StreamReader *reader;
};
}

#endif
