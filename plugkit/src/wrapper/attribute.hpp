#ifndef PLUGKIT_PROPERTY_WRAPPER_H
#define PLUGKIT_PROPERTY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Attr;

struct AttributeWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const Attr *prop);
  static const Attr *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);
  static NAN_GETTER(type);
  static NAN_SETTER(setType);

private:
  AttributeWrapper(Attr *prop);
  AttributeWrapper(const Attr *prop);
  AttributeWrapper(const AttributeWrapper &) = delete;
  AttributeWrapper &operator=(const AttributeWrapper &) = delete;

private:
  Attr *prop;
  const Attr *constProp;
};
}

#endif
