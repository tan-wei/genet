#ifndef PLUGKIT_LAYER_WRAPPER_H
#define PLUGKIT_LAYER_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Layer;

class LayerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Layer *layer);
  static const Layer *unwrapConst(v8::Local<v8::Object> obj);
  static Layer *unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(ns);
  static NAN_SETTER(setNs);
  static NAN_GETTER(streamId);
  static NAN_SETTER(setStreamId);
  static NAN_GETTER(summary);
  static NAN_SETTER(setSummary);
  static NAN_GETTER(confidence);
  static NAN_SETTER(setConfidence);
  static NAN_GETTER(error);
  static NAN_SETTER(setError);
  static NAN_GETTER(parent);
  static NAN_GETTER(children);
  static NAN_GETTER(properties);
  static NAN_GETTER(payload);
  static NAN_SETTER(setPayload);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(addChild);
  static NAN_METHOD(addProperty);
  static NAN_METHOD(toJSON);
  static NAN_GETTER(hasError);

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
