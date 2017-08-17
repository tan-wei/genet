#ifndef PLUGKIT_PAYLOAD_WRAPPER_H
#define PLUGKIT_PAYLOAD_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Payload;

struct PayloadWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const Payload *prop);
  static const Payload *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(data);
  static NAN_METHOD(addProperty);
  static NAN_GETTER(properties);
  static NAN_METHOD(propertyFromId);
  static NAN_GETTER(type);
  static NAN_SETTER(setType);

private:
  PayloadWrapper(Payload *prop);
  PayloadWrapper(const Payload *prop);
  PayloadWrapper(const PayloadWrapper &) = delete;
  PayloadWrapper &operator=(const PayloadWrapper &) = delete;

private:
  Payload *payload;
  const Payload *constPayload;
};
}

#endif
