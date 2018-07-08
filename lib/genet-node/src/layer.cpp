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
  Nan::SetAccessor(otl, Nan::New("attrs").ToLocalChecked(), attrs);
  Nan::SetAccessor(otl, Nan::New("payloads").ToLocalChecked(), payloads);
  Nan::SetAccessor(otl, Nan::New("data").ToLocalChecked(), data);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_LAYER);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Layer").ToLocalChecked(), ctor);
}

NAN_METHOD(LayerWrapper::New) {
  if (info.IsConstructCall()) {
    auto id = info[0];
    if (id->IsExternal()) {
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
    auto id =
        Nan::New(genet_token_string(genet_layer_id(layer))).ToLocalChecked();
    info.GetReturnValue().Set(id);
  }
}

NAN_METHOD(LayerWrapper::attr) {
  Token id = 0;
  if (info[0]->IsUint32()) {
    id = info[0]->Uint32Value();
  } else if (info[0]->IsString()) {
    Nan::Utf8String str(info[0]);
    id = genet_token_get(*str);
  } else {
    Nan::ThrowTypeError("First argument must be an integer or a string");
    return;
  }
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    if (const Attr *attr = genet_layer_attr(layer, id)) {
      info.GetReturnValue().Set(AttrWrapper::wrap(attr, layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_GETTER(LayerWrapper::attrs) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {

    uint32_t attrLength = 0;
    auto attrs = genet_layer_attrs(layer, &attrLength);
    uint32_t headerLength = 0;
    auto headers = genet_layer_headers(layer, &headerLength);

    uint32_t length = headerLength + attrLength;
    auto array = Nan::New<v8::Array>(length);

    for (uint32_t index = 0; index < headerLength; ++index) {
      array->Set(index, AttrWrapper::wrap(headers[index], layer));
    }
    for (uint32_t index = 0; index < attrLength; ++index) {
      array->Set(index + headerLength, AttrWrapper::wrap(attrs[index], layer));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::payloads) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    auto array = Nan::New<v8::Array>(0);
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::data) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer.getConst()) {
    uint64_t length = 0;
    auto data = genet_layer_data(layer, &length);
    info.GetReturnValue().Set(
        Nan::NewBuffer(data, length, [](char *data, void *hint) {}, nullptr)
            .ToLocalChecked());
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
