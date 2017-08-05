#ifndef PLUGKIT_SLICE_WRAPPER_H
#define PLUGKIT_SLICE_WRAPPER_H

#include <memory>
#include <nan.h>
#include "../slice.hpp"

namespace plugkit {

class Slice;

class SliceWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Slice &slice);
  static Slice unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(length);

private:
  SliceWrapper(const Slice &slice);
  SliceWrapper(const SliceWrapper &) = delete;
  SliceWrapper &operator=(const SliceWrapper &) = delete;

private:
  Slice slice;
};
}

#endif
