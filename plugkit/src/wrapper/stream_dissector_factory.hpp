#ifndef PLUGKIT_STREAM_DISSECTOR_FACTORY_WRAPPER_H
#define PLUGKIT_STREAM_DISSECTOR_FACTORY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class StreamDissectorFactory;
using StreamDissectorFactoryConstPtr =
    std::shared_ptr<const StreamDissectorFactory>;

class StreamDissectorFactoryWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object>
  wrap(const StreamDissectorFactoryConstPtr &factory);
  static StreamDissectorFactoryConstPtr unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);

private:
  StreamDissectorFactoryWrapper(const StreamDissectorFactoryConstPtr &factory);
  StreamDissectorFactoryWrapper(const StreamDissectorFactoryWrapper &) = delete;
  StreamDissectorFactoryWrapper &
  operator=(const StreamDissectorFactoryWrapper &) = delete;

private:
  StreamDissectorFactoryConstPtr factory;
};
}

#endif
