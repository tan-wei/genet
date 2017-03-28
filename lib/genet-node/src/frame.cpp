#include "frame.hpp"
#include "attr.hpp"
#include "exports.hpp"
#include "layer.hpp"
#include "module.hpp"

namespace genet_node {

void FrameWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Frame").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "query", query);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("index").ToLocalChecked(), index);
  Nan::SetAccessor(otl, Nan::New("layers").ToLocalChecked(), layers);
  Nan::SetAccessor(otl, Nan::New("treeIndices").ToLocalChecked(), treeIndices);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_FRAME);
  cls.ctor.Reset(isolate, ctor);
}

NAN_METHOD(FrameWrapper::New) {
  if (info.IsConstructCall()) {
    auto frame = info[0];
    if (frame->IsExternal()) {
      auto obj = static_cast<FrameWrapper *>(frame.As<v8::External>()->Value());
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }
}

NAN_GETTER(FrameWrapper::index) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (auto frame = wrapper->frame) {
    info.GetReturnValue().Set(genet_frame_index(frame));
  }
}

NAN_GETTER(FrameWrapper::layers) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (auto frame = wrapper->frame) {
    uint32_t length = 0;
    auto layers = genet_frame_layers(frame, &length);
    auto array = Nan::New<v8::Array>(length);
    for (uint32_t index = 0; index < length; ++index) {
      array->Set(index, LayerWrapper::wrap(Pointer<Layer>::ref(layers[index])));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(FrameWrapper::treeIndices) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (auto frame = wrapper->frame) {
    uint32_t length = 0;
    auto indices = genet_frame_tree_indices(frame, &length);
    auto array = Nan::New<v8::Array>(length);
    for (uint32_t index = 0; index < length; ++index) {
      array->Set(index, Nan::New(static_cast<uint32_t>(indices[index])));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(FrameWrapper::query) {
  static Token token_frame_index = genet_token_get("$.index");

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
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (auto frame = wrapper->frame) {
    if (id == token_frame_index) {
      info.GetReturnValue().Set(genet_frame_index(frame));
      return;
    }

    uint32_t length = 0;
    auto layers = genet_frame_layers(frame, &length);
    for (uint32_t i = 0; i < length; ++i) {
      const Layer *layer = layers[length - i - 1];
      if (genet_layer_id(layer) == id) {
        info.GetReturnValue().Set(
            LayerWrapper::wrap(Pointer<Layer>::ref(layer)));
        return;
      }
      if (const Attr *attr = genet_layer_attr(layer, id)) {
        info.GetReturnValue().Set(AttrWrapper::wrap(attr, layer));
        return;
      }
    }
    info.GetReturnValue().Set(Nan::Null());
  }
}

FrameWrapper::FrameWrapper(const Frame *frame) : frame(frame) {}

FrameWrapper::~FrameWrapper() {}

v8::Local<v8::Object> FrameWrapper::wrap(const Frame *frame) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  const auto &cls = Module::current().get(Module::CLASS_FRAME);
  auto cons = v8::Local<v8::Function>::New(isolate, cls.ctor);
  auto ptr = new FrameWrapper(frame);
  v8::Local<v8::Value> args[] = {Nan::New<v8::External>(ptr)};
  return Nan::NewInstance(cons, 1, args).ToLocalChecked();
}

} // namespace genet_node
