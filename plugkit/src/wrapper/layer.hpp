#ifndef PLUGKIT_LAYER_WRAPPER_H
#define PLUGKIT_LAYER_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

struct Layer;

class LayerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static v8::Local<v8::Object> wrap(Layer *layer);
  static v8::Local<v8::Object> wrap(const Layer *layer);
  static const Layer *unwrapConst(v8::Local<v8::Value> value);
  static Layer *unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(worker);
  static NAN_SETTER(setWorker);
  static NAN_GETTER(confidence);
  static NAN_SETTER(setConfidence);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(parent);
  static NAN_GETTER(prev);
  static NAN_GETTER(next);
  static NAN_GETTER(frame);
  static NAN_GETTER(layers);
  static NAN_GETTER(subLayers);
  static NAN_GETTER(attrs);
  static NAN_GETTER(payloads);
  static NAN_GETTER(errors);
  static NAN_GETTER(tags);
  static NAN_METHOD(attr);
  static NAN_METHOD(addLayer);
  static NAN_METHOD(addSubLayer);
  static NAN_METHOD(addPayload);
  static NAN_METHOD(addAttr);
  static NAN_METHOD(addAttrAlias);
  static NAN_METHOD(addAttrStr);
  static NAN_METHOD(addError);
  static NAN_METHOD(addTag);
  static NAN_METHOD(toJSON);

private:
  LayerWrapper(const Layer *layer);
  LayerWrapper(Layer *layer);
  LayerWrapper(const LayerWrapper &) = delete;
  LayerWrapper &operator=(const LayerWrapper &) = delete;

private:
  Layer *layer;
  const Layer *constLayer;
};
} // namespace plugkit

#endif
