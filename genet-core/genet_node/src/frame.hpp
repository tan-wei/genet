#ifndef PLUGNODE_FRAME_WRAPPER_H
#define PLUGNODE_FRAME_WRAPPER_H

#include <memory>
#include <nan.h>

struct Frame;

namespace genet_node {

class FrameWrapper final : public Nan::ObjectWrap {
 public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Frame* frame);
  static NAN_METHOD(New);
  static NAN_GETTER(index);

 public:
  ~FrameWrapper();

 private:
  FrameWrapper(const Frame* frame);
  FrameWrapper(const FrameWrapper&) = delete;
  FrameWrapper& operator=(const FrameWrapper&) = delete;

 private:
  const Frame* frame;
};

}  // namespace genet_node

#endif