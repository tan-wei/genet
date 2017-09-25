#include "../frame.hpp"
#include "../layer.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include "plugkit_module.hpp"
#include "wrapper/attribute.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

void LayerWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
  SetPrototypeMethod(tpl, "attr", attr);
  SetPrototypeMethod(tpl, "addLayer", addLayer);
  SetPrototypeMethod(tpl, "addSubLayer", addSubLayer);
  SetPrototypeMethod(tpl, "addAttr", addAttr);
  SetPrototypeMethod(tpl, "addTag", addTag);
  SetPrototypeMethod(tpl, "toJSON", toJSON);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("worker").ToLocalChecked(), worker, setWorker);
  Nan::SetAccessor(otl, Nan::New("confidence").ToLocalChecked(), confidence,
                   setConfidence);
  Nan::SetAccessor(otl, Nan::New("parent").ToLocalChecked(), parent);
  Nan::SetAccessor(otl, Nan::New("frame").ToLocalChecked(), frame);
  Nan::SetAccessor(otl, Nan::New("payloads").ToLocalChecked(), payloads);
  Nan::SetAccessor(otl, Nan::New("attrs").ToLocalChecked(), attrs);
  Nan::SetAccessor(otl, Nan::New("layers").ToLocalChecked(), layers);
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

NAN_METHOD(LayerWrapper::New) {
  if (!info[0]->IsObject()) {
    return;
  }
  auto options = info[0].As<v8::Object>();
  auto idValue = options->Get(Nan::New("id").ToLocalChecked());
  auto confValue = options->Get(Nan::New("confidence").ToLocalChecked());
  if (!idValue->IsNumber()) {
    return;
  }
  auto layer = new Layer(Token_get(*Nan::Utf8String(idValue)));
  if (confValue->IsNumber()) {
    layer->setConfidence(
        static_cast<LayerConfidence>(confValue->NumberValue()));
  }

  LayerWrapper *obj = new LayerWrapper(layer);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

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
    layer->setWorker(value->NumberValue());
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
    layer->setConfidence(static_cast<LayerConfidence>(value->NumberValue()));
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

NAN_GETTER(LayerWrapper::attrs) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &attrs = layer->attrs();
    auto array = v8::Array::New(isolate, attrs.size());
    for (size_t i = 0; i < attrs.size(); ++i) {
      array->Set(i, AttributeWrapper::wrap(attrs[i]));
    }
    info.GetReturnValue().Set(array);
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

NAN_METHOD(LayerWrapper::attr) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->constLayer) {
    Token token = Token_null();
    if (info[0]->IsNumber()) {
      token = info[0]->NumberValue();
    } else if (info[0]->IsString()) {
      token = Token_get(*Nan::Utf8String(info[0]));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }
    if (const auto &prop = layer->attr(token)) {
      info.GetReturnValue().Set(AttributeWrapper::wrap(prop));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(LayerWrapper::addLayer) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = info[0]->IsNumber() ? info[0]->NumberValue()
                                      : Token_get(*Nan::Utf8String(info[0]));
    info.GetReturnValue().Set(LayerWrapper::wrap(Layer_addLayer(layer, token)));
  }
}

NAN_METHOD(LayerWrapper::addSubLayer) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = info[0]->IsNumber() ? info[0]->NumberValue()
                                      : Token_get(*Nan::Utf8String(info[0]));
    info.GetReturnValue().Set(
        LayerWrapper::wrap(Layer_addSubLayer(layer, token)));
  }
}

NAN_METHOD(LayerWrapper::addAttr) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = Token_null();
    if (info[0]->IsNumber()) {
      token = info[0]->NumberValue();
    } else if (info[0]->IsString()) {
      token = Token_get(*Nan::Utf8String(info[0]));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }
    info.GetReturnValue().Set(
        AttributeWrapper::wrap(Layer_addAttr(layer, token)));
  }
}

NAN_METHOD(LayerWrapper::addTag) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    Token token = info[0]->IsNumber() ? info[0]->NumberValue()
                                      : Token_get(*Nan::Utf8String(info[0]));
    layer->addTag(token);
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

const Layer *LayerWrapper::unwrapConst(v8::Local<v8::Object> obj) {
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

Layer *LayerWrapper::unwrap(v8::Local<v8::Object> obj) {
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
