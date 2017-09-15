#ifndef PLUGKIT_LAYER_WRAPPER_H
#define PLUGKIT_LAYER_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Layer;

class LayerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Layer *layer);
  static const Layer *unwrapConst(v8::Local<v8::Object> obj);
  static Layer *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(worker);
  static NAN_SETTER(setWorker);
  static NAN_GETTER(confidence);
  static NAN_SETTER(setConfidence);
  static NAN_GETTER(parent);
  static NAN_GETTER(frame);
  static NAN_GETTER(children);
  static NAN_GETTER(properties);
  static NAN_GETTER(payloads);
  static NAN_GETTER(tags);
  static NAN_METHOD(attr);
  static NAN_METHOD(addLayer);
  static NAN_METHOD(addSubLayer);
  static NAN_METHOD(addAttr);
  static NAN_METHOD(addTag);
  static NAN_METHOD(toJSON);

private:
  LayerWrapper(const Layer *layer);
  LayerWrapper(Layer *layer);
  LayerWrapper(const LayerWrapper &) = delete;
  LayerWrapper &operator=(const LayerWrapper &) = delete;

private:
  const Layer *weakLayer;
  Layer *layer;
};
}

#endif
