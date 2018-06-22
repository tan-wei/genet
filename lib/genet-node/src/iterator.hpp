#ifndef PLUGNODE_ITERATOR_WRAPPER_H
#define PLUGNODE_ITERATOR_WRAPPER_H

#include <functional>
#include <memory>
#include <nan.h>

namespace genet_node {

class IteratorWrapper final : public Nan::ObjectWrap {
public:
  using Callback = v8::Local<v8::Value> (*)(void *, uint8_t *);

public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object>
  wrap(v8::Local<v8::Object> self, const Callback cb, void *data);
  static NAN_METHOD(New);
  static NAN_METHOD(next);

private:
  IteratorWrapper(const Callback func, void *data);
  IteratorWrapper(const IteratorWrapper &) = delete;
  IteratorWrapper &operator=(const IteratorWrapper &) = delete;

private:
  Callback cb;
  void *data = nullptr;
  uint8_t done = 0;
};
} // namespace genet_node

#endif
