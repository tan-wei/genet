#ifndef PLUGKIT_LISTENER_STATUS_WRAPPER_H
#define PLUGKIT_LISTENER_STATUS_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class ListenerStatus;

class ListenerStatusWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object>
  wrap(const std::shared_ptr<ListenerStatus> &status);
  static v8::Local<v8::Object>
  wrap(const std::weak_ptr<const ListenerStatus> &status);
  static std::shared_ptr<ListenerStatus> unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(attributes);
  static NAN_GETTER(chunks);
  static NAN_METHOD(getAttribute);
  static NAN_METHOD(getChunk);

private:
  ListenerStatusWrapper(const std::shared_ptr<ListenerStatus> &attr);
  ListenerStatusWrapper(const std::weak_ptr<const ListenerStatus> &attr);
  ListenerStatusWrapper(const ListenerStatusWrapper &) = delete;
  ListenerStatusWrapper &operator=(const ListenerStatusWrapper &) = delete;

private:
  std::shared_ptr<ListenerStatus> status;
  std::weak_ptr<const ListenerStatus> weakStatus;
};
}

#endif
