#ifndef PLUGKIT_CONTEXT_WRAPPER_H
#define PLUGKIT_CONTEXT_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Context;

class ContextWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static NAN_METHOD(New);
  static NAN_METHOD(getConfig);
  static NAN_METHOD(closeStream);
  static NAN_METHOD(assertConfidence);
  static NAN_METHOD(addLayerLinkage);
  static v8::Local<v8::Object> wrap(Context *ctx);
  static Context *unwrap(v8::Local<v8::Value> value);

private:
  ContextWrapper(Context *ctx);
  ContextWrapper(const ContextWrapper &) = delete;
  ContextWrapper &operator=(const ContextWrapper &) = delete;

private:
  Context *ctx;
};
} // namespace plugkit

#endif
