#ifndef PLUGKIT_LISTENER_FACTORY_WRAPPER_H
#define PLUGKIT_LISTENER_FACTORY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class ListenerFactory;
using ListenerFactoryConstPtr = std::shared_ptr<const ListenerFactory>;

class ListenerFactoryWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const ListenerFactoryConstPtr &factory);
  static ListenerFactoryConstPtr unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);

private:
  ListenerFactoryWrapper(const ListenerFactoryConstPtr &factory);
  ListenerFactoryWrapper(const ListenerFactoryWrapper &) = delete;
  ListenerFactoryWrapper &operator=(const ListenerFactoryWrapper &) = delete;

private:
  ListenerFactoryConstPtr factory;
};
}

#endif
