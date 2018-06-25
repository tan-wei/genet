#ifndef GENET_NODE_LAYER_WRAPPER_H
#define GENET_NODE_LAYER_WRAPPER_H

#include "pointer.hpp"
#include <memory>
#include <nan.h>

struct Layer;

namespace genet_node {

class LayerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Pointer<Layer> &layer);
  static Pointer<Layer> unwrap(v8::Local<v8::Value> value);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_METHOD(attr);
  static NAN_GETTER(attrs);
  static NAN_GETTER(data);

public:
  ~LayerWrapper();

private:
  LayerWrapper(const Pointer<Layer> &layer);
  LayerWrapper(const LayerWrapper &) = delete;
  LayerWrapper &operator=(const LayerWrapper &) = delete;

private:
  Pointer<Layer> layer;
};
} // namespace genet_node

#endif
