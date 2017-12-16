#include "plugkit_testing.hpp"
#include "allocator.hpp"
#include "attr.hpp"
#include "context.hpp"
#include "frame.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "null_logger.hpp"
#include "payload.hpp"
#include "token.h"
#include "wrapper/attr.hpp"
#include "wrapper/context.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/logger.hpp"
#include "wrapper/payload.hpp"
#include <nan.h>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

namespace plugkit {
namespace {
void runCApiTests(v8::FunctionCallbackInfo<v8::Value> const &info) {
  int result = Catch::Session().run();
  info.GetReturnValue().Set(result);
}
void externalize(v8::FunctionCallbackInfo<v8::Value> const &info) {
  if (info[0]->IsArrayBufferView()) {
    auto view = info[0].As<v8::ArrayBufferView>();
    auto buffer = view->Buffer();
    if (!buffer->IsExternal()) {
      buffer->Externalize();
      info.GetReturnValue().Set(view);
    }
  }
}
void createAttrInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  Token id = Token_get(*Nan::Utf8String(info[0]));
  auto attr = new Attr(id);
  Nan::Persistent<v8::Object> persistent(AttributeWrapper::wrap(attr));
  persistent.SetWeak(attr,
                     [](const Nan::WeakCallbackInfo<Attr> &data) {
                       delete data.GetParameter();
                     },
                     Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
void createContextInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  static RootAllocator allocator;
  auto ctx = new Context();
  ctx->rootAllocator = &allocator;
  Nan::Persistent<v8::Object> persistent(ContextWrapper::wrap(ctx));
  persistent.SetWeak(ctx,
                     [](const Nan::WeakCallbackInfo<Context> &data) {
                       delete data.GetParameter();
                     },
                     Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
void createPayloadInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  auto payload = new Payload();
  Nan::Persistent<v8::Object> persistent(PayloadWrapper::wrap(payload));
  persistent.SetWeak(payload,
                     [](const Nan::WeakCallbackInfo<Payload> &data) {
                       delete data.GetParameter();
                     },
                     Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
void createLayerInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  Token id = Token_get(*Nan::Utf8String(info[0]));
  auto frame = new Frame();
  new FrameView(frame);
  auto parent = new Layer(id);
  auto layer = new Layer(id);
  layer->setFrame(frame);
  layer->setParent(parent);
  Nan::Persistent<v8::Object> persistent(LayerWrapper::wrap(layer));
  persistent.SetWeak(layer,
                     [](const Nan::WeakCallbackInfo<Layer> &data) {
                       Layer *layer = data.GetParameter();
                       delete layer->frame()->view();
                       delete layer->frame();
                       delete layer->parent();
                       delete layer;
                     },
                     Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
void createFrameInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  auto view = new FrameView(new Frame());
  Nan::Persistent<v8::Object> persistent(FrameWrapper::wrap(view));
  persistent.SetWeak(view,
                     [](const Nan::WeakCallbackInfo<FrameView> &data) {
                       FrameView *view = data.GetParameter();
                       delete view->frame();
                       delete view;
                     },
                     Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
} // namespace
void PlugkitTesting::init(v8::Local<v8::Object> exports) {
  auto isolate = v8::Isolate::GetCurrent();
  auto testing = Nan::New<v8::Object>();
  exports->Set(Nan::New("Testing").ToLocalChecked(), testing);
  testing->Set(Nan::New("console").ToLocalChecked(),
               LoggerWrapper::wrap(std::make_shared<NullLogger>()));
  testing->Set(Nan::New("runCApiTests").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, runCApiTests)->GetFunction());
  testing->Set(Nan::New("externalize").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, externalize)->GetFunction());
  testing->Set(
      Nan::New("createAttrInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createAttrInstance)->GetFunction());
  testing->Set(
      Nan::New("createContextInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createContextInstance)->GetFunction());
  testing->Set(
      Nan::New("createPayloadInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createPayloadInstance)->GetFunction());
  testing->Set(
      Nan::New("createLayerInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createLayerInstance)->GetFunction());
  testing->Set(
      Nan::New("createFrameInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createFrameInstance)->GetFunction());
}
} // namespace plugkit
