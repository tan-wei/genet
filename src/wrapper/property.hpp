#ifndef PLUGKIT_PROPERTY_WRAPPER_H
#define PLUGKIT_PROPERTY_WRAPPER_H

#include <nan.h>
#include <memory>

namespace plugkit {

class Property;

class PropertyWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object>
  wrap(const std::shared_ptr<const Property> &prop);
  static std::shared_ptr<const Property> unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(name);
  static NAN_SETTER(setName);
  static NAN_GETTER(id);
  static NAN_SETTER(setId);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(summary);
  static NAN_SETTER(setSummary);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);
  static NAN_GETTER(properties);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(addProperty);

private:
  PropertyWrapper(const std::shared_ptr<Property> &prop);
  PropertyWrapper(const std::shared_ptr<const Property> &prop);
  PropertyWrapper(const PropertyWrapper &) = delete;
  PropertyWrapper &operator=(const PropertyWrapper &) = delete;

private:
  std::shared_ptr<Property> prop;
  std::shared_ptr<const Property> constProp;
};
}

#endif
