#ifndef GENET_NODE_ATTR_WRAPPER_H
#define GENET_NODE_ATTR_WRAPPER_H

#include <memory>
#include <nan.h>

struct Attr;

namespace genet_node {

class AttrWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> wrap(const Attr *frame);
  static NAN_METHOD(New);
  static NAN_GETTER(id);
  static NAN_GETTER(type);

public:
  ~AttrWrapper();

private:
  AttrWrapper(const Attr *frame);
  AttrWrapper(const AttrWrapper &) = delete;
  AttrWrapper &operator=(const AttrWrapper &) = delete;

private:
  const Attr *attr;
};

} // namespace genet_node

#endif