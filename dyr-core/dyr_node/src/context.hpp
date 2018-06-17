#ifndef PLUGNODE_CONTEXT_WRAPPER_H
#define PLUGNODE_CONTEXT_WRAPPER_H

#include "pointer.hpp"
#include <nan.h>

struct Context;

namespace dyr_node {

class ContextWrapper final : public Nan::ObjectWrap {
 public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Pointer<Context>& ctx);
  static NAN_METHOD(New);
  static NAN_METHOD(token);
  static NAN_METHOD(string);
  static NAN_METHOD(closeStream);
  static NAN_METHOD(getConfig);

 public:
  ~ContextWrapper();

 private:
  ContextWrapper(const Pointer<Context>& ctx);
  ContextWrapper(const ContextWrapper&) = delete;
  ContextWrapper& operator=(const ContextWrapper&) = delete;

 private:
  Pointer<Context> ctx;
};

}  // namespace dyr_node

#endif