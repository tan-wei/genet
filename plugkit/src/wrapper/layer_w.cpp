#include "../frame.hpp"
#include "../layer.hpp"
#include "iterator.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include "plugkit_module.hpp"
#include "wrapper/attr.hpp"
#include "wrapper/context.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

namespace {
  const auto errorTypeToken = Token_get("--error");
  const auto nextToken = Token_get("--next");
  const auto prevToken = Token_get("--prev");
}

void LayerWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
  SetPrototypeMethod(tpl, "prev", prev);
  SetPrototypeMethod(tpl, "next", next);
  SetPrototypeMethod(tpl, "attr", attr);
  SetPrototypeMethod(tpl, "addLayer", addLayer);
  SetPrototypeMethod(tpl, "addSubLayer", addSubLayer);
  SetPrototypeMethod(tpl, "addPayload", addPayload);
  SetPrototypeMethod(tpl, "addError", addError);
  SetPrototypeMethod(tpl, "addAttr", addAttr);
  SetPrototypeMethod(tpl, "addAttrAlias", addAttrAlias);
  SetPrototypeMethod(tpl, "addTag", addTag);
  SetPrototypeMethod(tpl, "toJSON", toJSON);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("worker").ToLocalChecked(), worker, setWorker);
  Nan::SetAccessor(otl, Nan::New("confidence").ToLocalChecked(), confidence,
                   setConfidence);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("parent").ToLocalChecked(), parent);
  Nan::SetAccessor(otl, Nan::New("frame").ToLocalChecked(), frame);
  Nan::SetAccessor(otl, Nan::New("payloads").ToLocalChecked(), payloads);
  Nan::SetAccessor(otl, Nan::New("errors").ToLocalChecked(), errors);
  Nan::SetAccessor(otl, Nan::New("attrs").ToLocalChecked(), attrs);
  Nan::SetAccessor(otl, Nan::New("layers").ToLocalChecked(), layers);
  Nan::SetAccessor(otl, Nan::New("subLayers").ToLocalChecked(), layers);
  Nan::SetAccessor(otl, Nan::New("tags").ToLocalChecked(), tags);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->layer.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->layer.ctor.Reset(isolate, ctor);
}

LayerWrapper::LayerWrapper(const Layer *layer)
    : layer(nullptr), constLayer(layer) {}

LayerWrapper::LayerWrapper(Layer *layer) : layer(layer), constLayer(layer) {}

NAN_METHOD(LayerWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(LayerWrapper::id) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(layer->id())).ToLocalChecked());
  }
}

NAN_GETTER(LayerWrapper::worker) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    info.GetReturnValue().Set(layer->worker());
  }
}

NAN_SETTER(LayerWrapper::setWorker) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setWorker(value->Uint32Value());
  }
}

NAN_GETTER(LayerWrapper::confidence) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    info.GetReturnValue().Set(layer->confidence());
  }
}

NAN_SETTER(LayerWrapper::setConfidence) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setConfidence(static_cast<LayerConfidence>(value->Uint32Value()));
  }
}

NAN_GETTER(LayerWrapper::range) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(static_cast<uint32_t>(layer->range().begin)));
    array->Set(1, Nan::New(static_cast<uint32_t>(layer->range().end)));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(LayerWrapper::setRange) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        layer->setRange(
            Range{array->Get(0)->Uint32Value(), array->Get(1)->Uint32Value()});
      }
    }
  }
}

NAN_GETTER(LayerWrapper::parent) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    if (auto parent = layer->parent()) {
      info.GetReturnValue().Set(LayerWrapper::wrap(parent));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_GETTER(LayerWrapper::frame) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    if (auto frame = layer->frame()) {
      if (auto view = frame->view()) {
        info.GetReturnValue().Set(FrameWrapper::wrap(view));
        return;
      }
    }
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(LayerWrapper::payloads) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &payloads = layer->payloads();
    auto array = v8::Array::New(isolate, payloads.size());
    for (size_t i = 0; i < payloads.size(); ++i) {
      array->Set(i, PayloadWrapper::wrap(payloads[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::errors) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    info.GetReturnValue().Set(IteratorWrapper::wrap(
        [](const void *data, size_t *index,
           bool *done) -> v8::Local<v8::Value> {
          const Layer *layer = static_cast<const Layer *>(data);
          const auto &attrs = layer->attrs();
          for (; *index < attrs.size() && attrs[*index]->type() != errorTypeToken; (*index)++);
          if (*index < attrs.size()) {
            return AttrWrapper::wrap(attrs[(*index)++]);
          }
          *done = true;
          return Nan::Null();
        },
        layer));
  }
}

NAN_GETTER(LayerWrapper::layers) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &layers = layer->layers();
    auto array = v8::Array::New(isolate, layers.size());
    for (size_t i = 0; i < layers.size(); ++i) {
      array->Set(i, LayerWrapper::wrap(layers[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::subLayers) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &layers = layer->subLayers();
    auto array = v8::Array::New(isolate, layers.size());
    for (size_t i = 0; i < layers.size(); ++i) {
      array->Set(i, LayerWrapper::wrap(layers[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::attrs) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    info.GetReturnValue().Set(IteratorWrapper::wrap(
        [](const void *data, size_t *index,
           bool *done) -> v8::Local<v8::Value> {
          const Layer *layer = static_cast<const Layer *>(data);
          const auto &attrs = layer->attrs();
          if (*index < attrs.size()) {
            return AttrWrapper::wrap(attrs[(*index)++]);
          }
          *done = true;
          return Nan::Null();
        },
        layer));
  }
}

NAN_GETTER(LayerWrapper::tags) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &tags = layer->tags();
    auto array = v8::Array::New(isolate, tags.size());
    for (size_t i = 0; i < tags.size(); ++i) {
      array->Set(i, Nan::New(Token_string(tags[i])).ToLocalChecked());
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(LayerWrapper::prev) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    const Attr *prev = nullptr;
    for (const Attr *attr : layer->attrs()) {
      if (attr->type() == prevToken) {
        prev = attr;
        break;
      }
    }
    if (prev) {
      const Layer *layer = static_cast<const Layer*>(prev->value().address());
      info.GetReturnValue().Set(LayerWrapper::wrap(layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(LayerWrapper::next) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    const Attr *next = nullptr;
    for (const Attr *attr : layer->attrs()) {
      if (attr->type() == nextToken) {
        next = attr;
        break;
      }
    }
    if (next) {
      const Layer *layer = static_cast<const Layer*>(next->value().address());
      info.GetReturnValue().Set(LayerWrapper::wrap(layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(LayerWrapper::attr) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    Token token = Token_null();
    auto id = info[0];
    if (id->IsUint32()) {
      token = id->Uint32Value();
    } else if (id->IsString()) {
      token = Token_get(*Nan::Utf8String(id));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }
    if (const auto &attr = layer->attr(token)) {
      info.GetReturnValue().Set(AttrWrapper::wrap(attr));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(LayerWrapper::addLayer) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    auto id = info[1];
    if (id->IsUint32()) {
      token = id->Uint32Value();
    } else if (id->IsString()) {
      token = Token_get(*Nan::Utf8String(id));
    } else {
      Nan::ThrowTypeError("Second argument must be a string or token-id");
      return;
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      info.GetReturnValue().Set(
          LayerWrapper::wrap(Layer_addLayer(layer, ctx, token)));
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addSubLayer) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    auto id = info[1];
    if (id->IsUint32()) {
      token = id->Uint32Value();
    } else if (id->IsString()) {
      token = Token_get(*Nan::Utf8String(id));
    } else {
      Nan::ThrowTypeError("Second argument must be a string or token-id");
      return;
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      info.GetReturnValue().Set(
          LayerWrapper::wrap(Layer_addSubLayer(layer, ctx, token)));
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addError) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    auto id = info[1];
    if (id->IsUint32()) {
      token = id->Uint32Value();
    } else if (id->IsString()) {
      token = Token_get(*Nan::Utf8String(id));
    } else {
      Nan::ThrowTypeError("Second argument must be a string or token-id");
      return;
    }
    const char *msg = nullptr;
    size_t length = 0;
    if (info[2]->IsString()) {
      const auto &str = Nan::Utf8String(info[2]);
      msg = *str;
      length = str.length();
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      Layer_addError(layer, ctx, token, msg, length);
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addPayload) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      info.GetReturnValue().Set(
          PayloadWrapper::wrap(Layer_addPayload(layer, ctx)));
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addAttr) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    auto id = info[1];
    if (id->IsUint32()) {
      token = id->Uint32Value();
    } else if (id->IsString()) {
      token = Token_get(*Nan::Utf8String(id));
    } else {
      Nan::ThrowTypeError("Second argument must be a string or token-id");
      return;
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      info.GetReturnValue().Set(
          AttrWrapper::wrap(Layer_addAttr(layer, ctx, token)));
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addAttrAlias) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token alias = Token_null();
    Token target = Token_null();
    auto aliasId = info[1];
    auto targetId = info[2];
    if (aliasId->IsUint32()) {
      alias = aliasId->Uint32Value();
    } else if (aliasId->IsString()) {
      alias = Token_get(*Nan::Utf8String(aliasId));
    } else {
      Nan::ThrowTypeError("Second argument must be a string or token-id");
      return;
    }
    if (targetId->IsUint32()) {
      target = targetId->Uint32Value();
    } else if (targetId->IsString()) {
      target = Token_get(*Nan::Utf8String(targetId));
    } else {
      Nan::ThrowTypeError("Third argument must be a string or token-id");
      return;
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      Layer_addAttrAlias(layer, ctx, alias, target);
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::addTag) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    if (info[1]->IsUint32()) {
      token = info[1]->Uint32Value();
    } else if (info[1]->IsString()) {
      token = Token_get(*Nan::Utf8String(info[1]));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }
    if (auto ctx = ContextWrapper::unwrap(info[0])) {
      Layer_addTag(layer, ctx, token);
    } else {
      Nan::ThrowTypeError("First argument must be a context");
    }
  }
}

NAN_METHOD(LayerWrapper::toJSON) {
  auto keys = info.This()->GetOwnPropertyNames();
  auto obj = Nan::New<v8::Object>();
  for (size_t i = 0; i < keys->Length(); ++i) {
    auto key = keys->Get(i).As<v8::String>();
    if (strcmp(*Nan::Utf8String(key), "parent") != 0) {
      obj->Set(key, info.This()->Get(key));
    }
  }
  info.GetReturnValue().Set(obj);
}

v8::Local<v8::Object> LayerWrapper::wrap(Layer *layer) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->layer.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  LayerWrapper *wrapper = new LayerWrapper(layer);
  wrapper->Wrap(obj);
  return obj;
}

v8::Local<v8::Object> LayerWrapper::wrap(const Layer *layer) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->layer.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  LayerWrapper *wrapper = new LayerWrapper(layer);
  wrapper->Wrap(obj);
  return obj;
}

const Layer *LayerWrapper::unwrapConst(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->layer.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<LayerWrapper>(obj)) {
      return wrapper->constLayer;
    }
  }
  return nullptr;
}

Layer *LayerWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->layer.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<LayerWrapper>(obj)) {
      return wrapper->layer;
    }
  }
  return nullptr;
}
} // namespace plugkit
