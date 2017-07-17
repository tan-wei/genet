#ifndef PLUGKIT_PROPERTY_WRAPPER_H
#define PLUGKIT_PROPERTY_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Property;

class PropertyWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Property *prop);
  static const Property *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_SETTER(setId);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(summary);
  static NAN_SETTER(setSummary);
  static NAN_GETTER(error);
  static NAN_SETTER(setError);
  static NAN_GETTER(value);
  static NAN_SETTER(setValue);
  static NAN_GETTER(properties);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(addProperty);

private:
  PropertyWrapper(Property *prop);
  PropertyWrapper(const Property *prop);
  PropertyWrapper(const PropertyWrapper &) = delete;
  PropertyWrapper &operator=(const PropertyWrapper &) = delete;

private:
  Property *prop;
  const Property *constProp;
};
}

#endif
