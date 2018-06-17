#include "layer.hpp"
#include "dyr_kernel.hpp"
#include "module.hpp"

namespace dyr_node {

namespace {
int marker = 0;
}

void LayerWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(2);
  tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "addChild", addChild);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("tags").ToLocalChecked(), tags);

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto& cls = Module::current().get(Module::CLASS_LAYER);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Layer").ToLocalChecked(), ctor);
}

NAN_METHOD(LayerWrapper::New) {
  if (info.IsConstructCall()) {
    auto id = info[0];
    if (id->IsUint32()) {
      LayerWrapper* obj = new LayerWrapper(
          Pointer<Layer>::owned(plug_layer_new(id->Uint32Value())));
      obj->Wrap(info.This());
    } else if (id->IsExternal()) {
      auto obj = static_cast<LayerWrapper*>(id.As<v8::External>()->Value());
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
  LayerWrapper* wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    info.GetReturnValue().Set(plug_layer_id(layer));
  } else {
    Nan::ThrowReferenceError("The layer has been moved");
  }
}

NAN_METHOD(LayerWrapper::addChild) {
  LayerWrapper* wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.get()) {
    auto child = LayerWrapper::unwrap(info[0]).get();
    if (child && layer != child) {
      ObjectWrap::Unwrap<LayerWrapper>(info[0].As<v8::Object>())->layer =
          Pointer<Layer>::null();
      plug_layer_add_child_move(layer, child);
    } else {
      Nan::ThrowTypeError("First argument should be a layer");
    }
  } else {
    Nan::ThrowTypeError("The layer is read-only");
  }
}

NAN_GETTER(LayerWrapper::tags) {
  LayerWrapper* wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    auto iter = plug_layer_tags(layer);
    auto array = Nan::New<v8::Array>();
    Token token;
    for (uint32_t i = 0; plug_layer_tags_next(iter, &token); ++i) {
      array->Set(i, Nan::New(token));
    }
    info.GetReturnValue().Set(array);
  }
}

LayerWrapper::LayerWrapper(const Pointer<Layer>& layer) : layer(layer) {}

LayerWrapper::~LayerWrapper() {
  plug_layer_free(layer.getOwned());
}

v8::Local<v8::Object> LayerWrapper::wrap(const Pointer<Layer>& layer) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  const auto& cls = Module::current().get(Module::CLASS_LAYER);
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

}  // namespace dyr_node
