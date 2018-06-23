#include "layer.hpp"
#include "attr.hpp"
#include "exports.hpp"
#include "module.hpp"

namespace genet_node {

namespace {
int marker = 0;
}

void LayerWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(2);
  tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "attr", attr);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_LAYER);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Layer").ToLocalChecked(), ctor);
}

NAN_METHOD(LayerWrapper::New) {
  if (info.IsConstructCall()) {
    auto id = info[0];
    if (id->IsUint32()) {
      LayerWrapper *obj = new LayerWrapper(
          Pointer<Layer>::owned(genet_layer_new(id->Uint32Value())));
      obj->Wrap(info.This());
    } else if (id->IsExternal()) {
      auto obj = static_cast<LayerWrapper *>(id.As<v8::External>()->Value());
      obj->Wrap(info.This());
    } else {
      Nan::ThrowTypeError("First argument must be a token-id");
      return;
    }
    Nan::SetInternalFieldPointer(info.This(), 1, &marker);
    info.GetReturnValue().Set(info.This());
  }
}

NAN_GETTER(LayerWrapper::id) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    info.GetReturnValue().Set(genet_layer_id(layer));
  }
}

NAN_METHOD(LayerWrapper::attr) {
  Token id = 0;
  if (info[0]->IsUint32()) {
    id = info[0]->Uint32Value();
  } else {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    if (const Attr *attr = genet_layer_attr(layer, id)) {
      info.GetReturnValue().Set(AttrWrapper::wrap(attr));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

LayerWrapper::LayerWrapper(const Pointer<Layer> &layer) : layer(layer) {}

LayerWrapper::~LayerWrapper() {}

v8::Local<v8::Object> LayerWrapper::wrap(const Pointer<Layer> &layer) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  const auto &cls = Module::current().get(Module::CLASS_LAYER);
  auto cons = v8::Local<v8::Function>::New(isolate, cls.ctor);
  auto ptr = new LayerWrapper(layer);
  v8::Local<v8::Value> args[] = {Nan::New<v8::External>(ptr)};
  return Nan::NewInstance(cons, 1, args).ToLocalChecked();
}

Pointer<Layer> LayerWrapper::unwrap(v8::Local<v8::Value> value) {
  if (!value.IsEmpty() && value->IsObject()) {
    auto object = value.As<v8::Object>();
    if (Nan::GetInternalFieldPointer(object, 1) == &marker) {
      if (auto wrapper = Nan::ObjectWrap::Unwrap<LayerWrapper>(object)) {
        return wrapper->layer;
      }
    }
  }
  return Pointer<Layer>::null();
}

} // namespace genet_node
