#include "layer.hpp"
#include "../plugkit/layer.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"
#include "wrapper/property.hpp"

namespace plugkit {

void LayerWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Layer").ToLocalChecked());
  SetPrototypeMethod(tpl, "propertyFromId", propertyFromId);
  SetPrototypeMethod(tpl, "addProperty", addProperty);
  SetPrototypeMethod(tpl, "addChild", addChild);
  SetPrototypeMethod(tpl, "toJSON", toJSON);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("namespace").ToLocalChecked(), ns, setNs);
  Nan::SetAccessor(otl, Nan::New("summary").ToLocalChecked(), summary,
                   setSummary);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);
  Nan::SetAccessor(otl, Nan::New("confidence").ToLocalChecked(), confidence,
                   setConfidence);
  Nan::SetAccessor(otl, Nan::New("error").ToLocalChecked(), error, setError);
  Nan::SetAccessor(otl, Nan::New("parent").ToLocalChecked(), parent);
  Nan::SetAccessor(otl, Nan::New("payload").ToLocalChecked(), payload,
                   setPayload);
  Nan::SetAccessor(otl, Nan::New("properties").ToLocalChecked(), properties);
  Nan::SetAccessor(otl, Nan::New("children").ToLocalChecked(), children);
  Nan::SetAccessor(otl, Nan::New("hasError").ToLocalChecked(), hasError);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->layer.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->layer.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Layer").ToLocalChecked(), func);
}

LayerWrapper::LayerWrapper(const Layer *layer) : weakLayer(layer) {}

LayerWrapper::LayerWrapper(Layer *layer) : weakLayer(layer), layer(layer) {}

NAN_METHOD(LayerWrapper::New) {
  // TODO:ALLOC
  auto layer = new Layer();

  if (info[0]->IsObject()) {
    auto options = info[0].As<v8::Object>();
    auto nsValue = options->Get(Nan::New("namespace").ToLocalChecked());
    auto summaryValue = options->Get(Nan::New("summary").ToLocalChecked());
    auto rangeValue = options->Get(Nan::New("range").ToLocalChecked());
    auto confValue = options->Get(Nan::New("confidence").ToLocalChecked());
    auto errorValue = options->Get(Nan::New("error").ToLocalChecked());
    if (nsValue->IsString()) {
      layer->setNs(strns(*Nan::Utf8String(nsValue)));
    }
    if (summaryValue->IsString()) {
      layer->setSummary(*Nan::Utf8String(summaryValue));
    }
    if (rangeValue->IsArray()) {
      auto array = rangeValue.As<v8::Array>();
      if (array->Length() >= 2) {
        layer->setRange(std::make_pair(array->Get(0)->Uint32Value(),
                                       array->Get(1)->Uint32Value()));
      }
    }
    if (confValue->IsNumber()) {
      layer->setConfidence(confValue->NumberValue());
    }
    if (errorValue->IsString()) {
      layer->setError(*Nan::Utf8String(errorValue));
    }
  }

  LayerWrapper *obj = new LayerWrapper(layer);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(LayerWrapper::id) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(Nan::New(layer->id().str()).ToLocalChecked());
  }
}

NAN_GETTER(LayerWrapper::ns) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(Nan::New(layer->ns().str()).ToLocalChecked());
  }
}

NAN_SETTER(LayerWrapper::setNs) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setNs(strns(*Nan::Utf8String(value)));
  }
}

NAN_GETTER(LayerWrapper::summary) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(Nan::New(layer->summary()).ToLocalChecked());
  }
}

NAN_SETTER(LayerWrapper::setSummary) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setSummary(*Nan::Utf8String(value));
  }
}

NAN_GETTER(LayerWrapper::range) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(layer->range().first));
    array->Set(1, Nan::New(layer->range().second));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(LayerWrapper::setRange) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        layer->setRange(std::make_pair(array->Get(0)->Uint32Value(),
                                       array->Get(1)->Uint32Value()));
      }
    }
  }
}

NAN_GETTER(LayerWrapper::confidence) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(layer->confidence());
  }
}

NAN_SETTER(LayerWrapper::setConfidence) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setConfidence(value->NumberValue());
  }
}

NAN_GETTER(LayerWrapper::error) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(Nan::New(layer->error()).ToLocalChecked());
  }
}

NAN_SETTER(LayerWrapper::setError) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    layer->setError(*Nan::Utf8String(value));
  }
}

NAN_GETTER(LayerWrapper::parent) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    if (auto parent = layer->parent()) {
      info.GetReturnValue().Set(LayerWrapper::wrap(parent));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_GETTER(LayerWrapper::payload) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(
        Variant::Private::getNodeBuffer(layer->payload()));
  }
}

NAN_SETTER(LayerWrapper::setPayload) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (value->IsObject()) {
      layer->setPayload(Variant::Private::getSlice(value.As<v8::Object>()));
    }
  }
}

NAN_GETTER(LayerWrapper::children) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &children = layer->children();
    auto array = v8::Array::New(isolate, children.size());
    for (size_t i = 0; i < children.size(); ++i) {
      array->Set(i, LayerWrapper::wrap(children[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(LayerWrapper::properties) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &properties = layer->properties();
    auto array = v8::Array::New(isolate, properties.size());
    for (size_t i = 0; i < properties.size(); ++i) {
      array->Set(i, PropertyWrapper::wrap(properties[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(LayerWrapper::propertyFromId) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    if (const auto &prop =
            layer->propertyFromId(strid(*Nan::Utf8String(info[0])))) {
      info.GetReturnValue().Set(PropertyWrapper::wrap(prop));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(LayerWrapper::addChild) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (info[0]->IsObject()) {
      if (const auto &child = LayerWrapper::unwrap(info[0].As<v8::Object>())) {
        layer->addChild(child);
      }
    }
  }
}

NAN_METHOD(LayerWrapper::addProperty) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->layer) {
    if (info[0]->IsObject()) {
      if (const auto &prop =
              PropertyWrapper::unwrap(info[0].As<v8::Object>())) {
        layer->addProperty(prop);
      }
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

NAN_GETTER(LayerWrapper::hasError) {
  LayerWrapper *wrapper = ObjectWrap::Unwrap<LayerWrapper>(info.Holder());
  if (auto layer = wrapper->weakLayer) {
    info.GetReturnValue().Set(layer->hasError());
  }
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
      return wrapper->weakLayer;
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
}
