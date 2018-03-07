#ifndef PLUGKIT_ITERATOR_WRAPPER_H
#define PLUGKIT_ITERATOR_WRAPPER_H

#include <functional>
#include <memory>
#include <nan.h>

namespace plugkit {

class IteratorWrapper final : public Nan::ObjectWrap {
public:
  using Callback = v8::Local<v8::Value> (*)(const void *, size_t *, bool *);

public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const Callback cb, const void *data);
  static NAN_METHOD(New);
  static NAN_METHOD(next);

private:
  IteratorWrapper(const Callback func, const void *data);
  IteratorWrapper(const IteratorWrapper &) = delete;
  IteratorWrapper &operator=(const IteratorWrapper &) = delete;

private:
  Callback cb;
  const void *data = nullptr;
  size_t index = 0;
  bool done = false;
};
} // namespace plugkit

#endif
