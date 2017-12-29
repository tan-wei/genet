#ifndef PLUGKIT_PROPERTY_WRAPPER_H
#define PLUGKIT_PROPERTY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Attr;

struct AttributeWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(Attr *attr);
  static v8::Local<v8::Object> wrap(const Attr *attr);
  static const Attr *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);
  static NAN_GETTER(type);
  static NAN_SETTER(setType);
  static NAN_GETTER(error);
  static NAN_SETTER(setError);

private:
  AttributeWrapper(Attr *attr);
  AttributeWrapper(const Attr *attr);
  AttributeWrapper(const AttributeWrapper &) = delete;
  AttributeWrapper &operator=(const AttributeWrapper &) = delete;

private:
  Attr *attr;
  const Attr *constProp;
};
} // namespace plugkit

#endif
