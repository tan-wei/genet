#include "chunk.hpp"
#include "../plugkit/chunk.hpp"
#include "extended_slot.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/property.hpp"

namespace plugkit {

void ChunkWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Chunk").ToLocalChecked());
  SetPrototypeMethod(tpl, "propertyFromId", propertyFromId);
  SetPrototypeMethod(tpl, "addProperty", addProperty);
  SetPrototypeMethod(tpl, "toJSON", toJSON);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("streamNamespace").ToLocalChecked(), streamNs,
                   setStreamNs);
  Nan::SetAccessor(otl, Nan::New("streamId").ToLocalChecked(), streamId,
                   setStreamId);
  Nan::SetAccessor(otl, Nan::New("payload").ToLocalChecked(), payload,
                   setPayload);
  Nan::SetAccessor(otl, Nan::New("properties").ToLocalChecked(), properties);
  Nan::SetAccessor(otl, Nan::New("layer").ToLocalChecked(), layer);

  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->chunk.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->chunk.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Chunk").ToLocalChecked(), func);
}

ChunkWrapper::ChunkWrapper(const std::shared_ptr<Chunk> &chunk)
    : chunk(chunk), constChunk(chunk) {}

ChunkWrapper::ChunkWrapper(const std::shared_ptr<const Chunk> &chunk)
    : constChunk(chunk) {}

NAN_METHOD(ChunkWrapper::New) {
  ChunkWrapper *obj = new ChunkWrapper(std::make_shared<Chunk>());
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ChunkWrapper::streamNs) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    info.GetReturnValue().Set(Nan::New(chunk->streamNs()).ToLocalChecked());
  }
}

NAN_SETTER(ChunkWrapper::setStreamNs) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    chunk->setStreamNs(*Nan::Utf8String(value));
  }
}

NAN_GETTER(ChunkWrapper::streamId) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    info.GetReturnValue().Set(Nan::New(chunk->streamId()).ToLocalChecked());
  }
}

NAN_SETTER(ChunkWrapper::setStreamId) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    chunk->setStreamId(*Nan::Utf8String(value));
  }
}

NAN_GETTER(ChunkWrapper::payload) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    info.GetReturnValue().Set(
        Variant::Private::getNodeBuffer(chunk->payload()));
  }
}

NAN_SETTER(ChunkWrapper::setPayload) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    if (value->IsObject()) {
      chunk->setPayload(Variant::Private::getSlice(value.As<v8::Object>()));
    }
  }
}

NAN_GETTER(ChunkWrapper::properties) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &properties = chunk->properties();
    auto array = v8::Array::New(isolate, properties.size());
    for (size_t i = 0; i < properties.size(); ++i) {
      array->Set(i, PropertyWrapper::wrap(properties[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(ChunkWrapper::propertyFromId) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    if (const auto &prop = chunk->propertyFromId(*Nan::Utf8String(info[0]))) {
      info.GetReturnValue().Set(PropertyWrapper::wrap(prop));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(ChunkWrapper::addProperty) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    if (info[0]->IsObject()) {
      if (const auto &prop =
              PropertyWrapper::unwrap(info[0].As<v8::Object>())) {
        chunk->addProperty(prop);
      }
    }
  }
}

NAN_GETTER(ChunkWrapper::layer) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->constChunk) {
    if (const auto &layer = chunk->layer()) {
      info.GetReturnValue().Set(LayerWrapper::wrap(layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_METHOD(ChunkWrapper::toJSON) {
  auto keys = info.This()->GetOwnPropertyNames();
  auto obj = Nan::New<v8::Object>();
  for (size_t i = 0; i < keys->Length(); ++i) {
    auto key = keys->Get(i).As<v8::String>();
    if (strcmp(*Nan::Utf8String(key), "layer") != 0) {
      obj->Set(key, info.This()->Get(key));
    }
  }
  info.GetReturnValue().Set(obj);
}

v8::Local<v8::Object>
ChunkWrapper::wrap(const std::shared_ptr<const Chunk> &chunk) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto cons = v8::Local<v8::Function>::New(isolate, module->chunk.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ChunkWrapper *wrapper = new ChunkWrapper(chunk);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<const Chunk> ChunkWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->chunk.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ChunkWrapper>(obj)) {
      return wrapper->constChunk;
    }
  }
  return std::shared_ptr<Chunk>();
}
}
