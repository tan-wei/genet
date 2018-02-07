#ifndef PLUGKIT_PROPERTY_WRAPPER_H
#define PLUGKIT_PROPERTY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Attr;

struct AttrWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(Attr *attr);
  static v8::Local<v8::Object> wrap(const Attr *attr);
  static v8::Local<v8::Object> wrap(const std::shared_ptr<Attr> &attr);
  static const Attr *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_METHOD(create);
  static NAN_GETTER(id);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);
  static NAN_GETTER(type);
  static NAN_SETTER(setType);

private:
  AttrWrapper(Attr *attr);
  AttrWrapper(const Attr *attr);
  AttrWrapper(const std::shared_ptr<Attr> &attr);
  AttrWrapper(const AttrWrapper &) = delete;
  AttrWrapper &operator=(const AttrWrapper &) = delete;

private:
  Attr *attr;
  const Attr *constProp;
  std::shared_ptr<Attr> shared;
};
} // namespace plugkit

#endif
