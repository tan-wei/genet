#ifndef GENET_NODE_ATTR_WRAPPER_H
#define GENET_NODE_ATTR_WRAPPER_H

#include <memory>
#include <nan.h>

struct Attr;
struct Layer;

namespace genet_node {

class AttrWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Attr *attr, const Layer *layer);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(type);
  static NAN_GETTER(range);
  static NAN_METHOD(getValue);

public:
  ~AttrWrapper();

private:
  AttrWrapper(const Attr *attr, const Layer *layer);
  AttrWrapper(const AttrWrapper &) = delete;
  AttrWrapper &operator=(const AttrWrapper &) = delete;

private:
  const Attr *attr;
  const Layer *layer;
};

} // namespace genet_node

#endif