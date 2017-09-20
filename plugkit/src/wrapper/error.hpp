#ifndef PLUGKIT_ERROR_WRAPPER_H
#define PLUGKIT_ERROR_WRAPPER_H

#include <memory>
#include <nan.h>
#include "error.h"

namespace plugkit {

class ErrorWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static NAN_METHOD(New);
  static NAN_GETTER(target);
  static NAN_GETTER(type);
  static v8::Local<v8::Object> wrap(const Error &error);
  static Error unwrap(v8::Local<v8::Object> obj);

private:
  ErrorWrapper(const Error &error);
  ErrorWrapper(const ErrorWrapper &) = delete;
  ErrorWrapper &operator=(const ErrorWrapper &) = delete;

private:
  Error error;
};
}

#endif
