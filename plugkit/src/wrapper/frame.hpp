#ifndef PLUGKIT_FRAME_WRAPPER_H
#define PLUGKIT_FRAME_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class FrameView;
using FrameViewConstPtr = const FrameView *;

class FrameWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const FrameViewConstPtr &view);
  static const FrameView *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(timestamp);
  static NAN_GETTER(length);
  static NAN_GETTER(index);
  static NAN_GETTER(rootLayer);
  static NAN_GETTER(primaryLayer);
  static NAN_GETTER(leafLayers);
  static NAN_GETTER(hasError);
  static NAN_GETTER(sourceId);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(layerFromId);

private:
  FrameWrapper(const FrameViewConstPtr &view);
  FrameWrapper(const FrameWrapper &) = delete;
  FrameWrapper &operator=(const FrameWrapper &) = delete;

private:
  FrameViewConstPtr view;
};
}

#endif
