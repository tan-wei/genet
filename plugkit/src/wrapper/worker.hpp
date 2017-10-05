#ifndef PLUGKIT_WORKER_WRAPPER_H
#define PLUGKIT_WORKER_WRAPPER_H

#include "reader.h"
#include <memory>
#include <nan.h>

namespace plugkit {

class WorkerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static NAN_METHOD(New);
  static bool unwrap(v8::Local<v8::Object> obj);

private:
  WorkerWrapper();
  WorkerWrapper(const WorkerWrapper &) = delete;
  WorkerWrapper &operator=(const WorkerWrapper &) = delete;

private:
};
} // namespace plugkit

#endif
