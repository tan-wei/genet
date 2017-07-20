#ifndef PLUGKIT_ATTRIBUTE_WRAPPER_H
#define PLUGKIT_ATTRIBUTE_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Attribute;

class AttributeWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const std::shared_ptr<Attribute> &attr);
  static v8::Local<v8::Object> wrap(const std::weak_ptr<const Attribute> &attr);
  static std::shared_ptr<Attribute> unwrap(v8::Local<v8::Object> obj);
  static std::shared_ptr<const Attribute>
  unwrapConst(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_SETTER(setId);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);

private:
  AttributeWrapper(const std::shared_ptr<Attribute> &attr);
  AttributeWrapper(const std::weak_ptr<const Attribute> &attr);
  AttributeWrapper(const AttributeWrapper &) = delete;
  AttributeWrapper &operator=(const AttributeWrapper &) = delete;

private:
  std::shared_ptr<Attribute> attr;
  std::weak_ptr<const Attribute> weakAttr;
};
}

#endif
