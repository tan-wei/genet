#include "plugkit_testing.hpp"
#include "attribute.hpp"
#include "payload.hpp"
#include "token.h"
#include "wrapper/attribute.hpp"
#include "wrapper/payload.hpp"
#include "null_logger.hpp"
#include "wrapper/logger.hpp"
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
  persistent.SetWeak(attr, [](const Nan::WeakCallbackInfo<Attr> &data) {
    delete data.GetParameter();
  }, Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
void createPayloadInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  Token id = Token_get(*Nan::Utf8String(info[0]));
  auto payload = new Payload();
  Nan::Persistent<v8::Object> persistent(PayloadWrapper::wrap(payload));
  persistent.SetWeak(payload, [](const Nan::WeakCallbackInfo<Payload> &data) {
    delete data.GetParameter();
  }, Nan::WeakCallbackType::kParameter);
  info.GetReturnValue().Set(persistent);
}
} // namespace
void PlugkitTesting::init(v8::Local<v8::Object> exports) {
  auto isolate = v8::Isolate::GetCurrent();
  auto testing = Nan::New<v8::Object>();
  exports->Set(Nan::New("Testing").ToLocalChecked(), testing);
  testing->Set(Nan::New("console").ToLocalChecked(),
               LoggerWrapper::wrap(std::make_shared<NullLogger>()));
  testing->Set(
      Nan::New("runCApiTests").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, runCApiTests, exports)->GetFunction());
  testing->Set(
      Nan::New("externalize").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, externalize, exports)->GetFunction());
  testing->Set(Nan::New("createAttrInstance").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, createAttrInstance, exports)
                   ->GetFunction());
  testing->Set(
      Nan::New("createPayloadInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createPayloadInstance, exports)
          ->GetFunction());
}
} // namespace plugkit
