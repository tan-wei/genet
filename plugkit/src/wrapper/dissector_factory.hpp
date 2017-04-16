#ifndef PLUGKIT_DISSECTOR_FACTORY_WRAPPER_H
#define PLUGKIT_DISSECTOR_FACTORY_WRAPPER_H

#include <nan.h>
#include <memory>

namespace plugkit {

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class DissectorFactoryWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const DissectorFactoryConstPtr &factory);
  static DissectorFactoryConstPtr unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);

private:
  DissectorFactoryWrapper(const DissectorFactoryConstPtr &factory);
  DissectorFactoryWrapper(const DissectorFactoryWrapper &) = delete;
  DissectorFactoryWrapper &operator=(const DissectorFactoryWrapper &) = delete;

private:
  DissectorFactoryConstPtr factory;
};
}

#endif
