#ifndef PLUGKIT_PAYLOAD_WRAPPER_H
#define PLUGKIT_PAYLOAD_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Payload;

struct PayloadWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(Payload *payload);
  static v8::Local<v8::Object> wrap(const Payload *payload);
  static const Payload *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_METHOD(addSlice);
  static NAN_GETTER(slices);
  static NAN_GETTER(length);
  static NAN_METHOD(addAttr);
  static NAN_GETTER(attrs);
  static NAN_METHOD(attr);
  static NAN_GETTER(type);
  static NAN_SETTER(setType);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_INDEX_GETTER(indexGetter);

private:
  PayloadWrapper(Payload *payload);
  PayloadWrapper(const Payload *payload);
  PayloadWrapper(const PayloadWrapper &) = delete;
  PayloadWrapper &operator=(const PayloadWrapper &) = delete;

private:
  ;
  const Payload *constPayload;
};
} // namespace plugkit

#endif
