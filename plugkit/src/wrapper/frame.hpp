#ifndef PLUGKIT_FRAME_WRAPPER_H
#define PLUGKIT_FRAME_WRAPPER_H

#include "../attr.hpp"
#include <memory>
#include <nan.h>

namespace plugkit {

class FrameView;

class FrameWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const FrameView *view);
  static const FrameView *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_GETTER(rootLayer);
  static NAN_GETTER(primaryLayer);
  static NAN_GETTER(leafLayers);
  static NAN_GETTER(sourceId);
  static NAN_METHOD(query);

private:
  FrameWrapper(const FrameView *view);
  FrameWrapper(const FrameWrapper &) = delete;
  FrameWrapper &operator=(const FrameWrapper &) = delete;

private:
  const FrameView *view;

public:
  Attr tsAttr;
  Attr payloadAttr;
  Attr actLenAttr;
  Attr indexAttr;
};
} // namespace plugkit

#endif
