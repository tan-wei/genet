#include "chunk.hpp"
#include "../plugkit/chunk.hpp"
#include "plugkit_module.hpp"
#include "private/variant.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/chunk.hpp"

namespace plugkit {

void ChunkWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Chunk").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("layer").ToLocalChecked(), layer, setLayer);
  Nan::SetAccessor(otl, Nan::New("range").ToLocalChecked(), range, setRange);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->chunk.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->chunk.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Chunk").ToLocalChecked(), func);
}

ChunkWrapper::ChunkWrapper(const std::shared_ptr<Chunk> &chunk)
    : chunk(chunk), weakChunk(chunk) {}

ChunkWrapper::ChunkWrapper(const std::weak_ptr<Chunk> &chunk)
    : weakChunk(chunk) {}

NAN_METHOD(ChunkWrapper::New) {
  const Layer *layer = LayerWrapper::unwrap(info[0].As<v8::Object>());
  if (!layer)
    return;

  auto chunk = std::make_shared<Chunk>(layer);

  if (info[1]->IsArray()) {
    auto array = info[1].As<v8::Array>();
    if (array->Length() >= 2) {
      chunk->setRange(std::make_pair(array->Get(0)->Uint32Value(),
                                     array->Get(1)->Uint32Value()));
    }
  }

  ChunkWrapper *obj = new ChunkWrapper(chunk);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ChunkWrapper::layer) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->weakChunk.lock()) {
    info.GetReturnValue().Set(LayerWrapper::wrap(chunk->layer()));
  }
}

NAN_SETTER(ChunkWrapper::setLayer) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    if (const Layer *layer = LayerWrapper::unwrap(value.As<v8::Object>())) {
      chunk->setLayer(layer);
    }
  }
}

NAN_GETTER(ChunkWrapper::range) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->weakChunk.lock()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto array = v8::Array::New(isolate, 2);
    array->Set(0, Nan::New(chunk->range().first));
    array->Set(1, Nan::New(chunk->range().second));
    info.GetReturnValue().Set(array);
  }
}

NAN_SETTER(ChunkWrapper::setRange) {
  ChunkWrapper *wrapper = ObjectWrap::Unwrap<ChunkWrapper>(info.Holder());
  if (auto chunk = wrapper->chunk) {
    if (value->IsArray()) {
      auto array = value.As<v8::Array>();
      if (array->Length() >= 2) {
        chunk->setRange(std::make_pair(array->Get(0)->Uint32Value(),
                                       array->Get(1)->Uint32Value()));
      }
    }
  }
}

v8::Local<v8::Object> ChunkWrapper::wrap(const std::shared_ptr<Chunk> &chunk) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->chunk.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ChunkWrapper *wrapper = new ChunkWrapper(chunk);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<Chunk> ChunkWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->chunk.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ChunkWrapper>(obj)) {
      return wrapper->chunk;
    }
  }
  return nullptr;
}

std::shared_ptr<const Chunk>
ChunkWrapper::unwrapConst(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->chunk.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ChunkWrapper>(obj)) {
      return wrapper->weakChunk.lock();
    }
  }
  return nullptr;
}
}
