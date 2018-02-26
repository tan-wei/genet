#ifndef PLUGKIT_ERROR_WRAPPER_H
#define PLUGKIT_ERROR_WRAPPER_H

#include <nan.h>

namespace plugkit {

struct Error;

struct ErrorWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(Error *error);
  static v8::Local<v8::Object> wrap(const Error *error);
  static const Error *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(target);
  static NAN_SETTER(setTarget);
  static NAN_GETTER(message);
  static NAN_SETTER(setMessage);

private:
  ErrorWrapper(Error *error);
  ErrorWrapper(const Error *error);
  ErrorWrapper(const ErrorWrapper &) = delete;
  ErrorWrapper &operator=(const ErrorWrapper &) = delete;

private:
  Error *error;
  const Error *constError;
};
} // namespace plugkit

#endif
