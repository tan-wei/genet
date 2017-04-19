#ifndef PLUGKIT_FRAME_WRAPPER_H
#define PLUGKIT_FRAME_WRAPPER_H

#include <nan.h>
#include <memory>

namespace plugkit {

class FrameView;

class FrameWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(const std::weak_ptr<const FrameView> &view);
  static std::shared_ptr<const FrameView> unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(timestamp);
  static NAN_GETTER(length);
  static NAN_GETTER(seq);
  static NAN_GETTER(rootLayer);
  static NAN_GETTER(primaryLayer);
  static NAN_GETTER(leafLayers);
  static NAN_GETTER(hasError);

private:
  FrameWrapper(const std::weak_ptr<const FrameView> &view);
  FrameWrapper(const FrameWrapper &) = delete;
  FrameWrapper &operator=(const FrameWrapper &) = delete;

private:
  std::weak_ptr<const FrameView> view;
};
}

#endif
