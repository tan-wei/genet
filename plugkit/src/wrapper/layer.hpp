#ifndef PLUGKIT_LAYER_WRAPPER_H
#define PLUGKIT_LAYER_WRAPPER_H

#include <nan.h>
#include <memory>

namespace plugkit {

class Layer;
using LayerPtr = std::shared_ptr<Layer>;
using LayerConstPtr = std::shared_ptr<const Layer>;
using LayerConstWeakPtr = std::weak_ptr<const Layer>;

class LayerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate, v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const LayerConstWeakPtr &layer);
  static LayerConstPtr unwrapConst(v8::Local<v8::Object> obj);
  static LayerPtr unwrap(v8::Local<v8::Object> obj);
  static NAN_METHOD(New);
  static NAN_GETTER(ns);
  static NAN_SETTER(setNs);
  static NAN_GETTER(name);
  static NAN_SETTER(setName);
  static NAN_GETTER(summary);
  static NAN_SETTER(setSummary);
  static NAN_GETTER(range);
  static NAN_SETTER(setRange);
  static NAN_GETTER(confidence);
  static NAN_SETTER(setConfidence);
  static NAN_GETTER(parent);
  static NAN_GETTER(children);
  static NAN_GETTER(properties);
  static NAN_GETTER(chunks);
  static NAN_GETTER(payload);
  static NAN_SETTER(setPayload);
  static NAN_METHOD(propertyFromId);
  static NAN_METHOD(addChild);
  static NAN_METHOD(addProperty);
  static NAN_METHOD(addChunk);
  static NAN_METHOD(toJSON);

private:
  LayerWrapper(const LayerPtr &layer);
  LayerWrapper(const LayerConstWeakPtr &layer);
  LayerWrapper(const LayerWrapper &) = delete;
  LayerWrapper &operator=(const LayerWrapper &) = delete;

private:
  LayerConstWeakPtr weakLayer;
  LayerPtr layer;
};
}

#endif
