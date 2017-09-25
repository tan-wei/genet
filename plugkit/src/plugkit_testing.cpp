#include "plugkit_testing.hpp"
#include "token.h"
#include "attribute.hpp"
#include "wrapper/attribute.hpp"
#include "payload.hpp"
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
void createAttrInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  Token id = Token_get(*Nan::Utf8String(info[0]));
  auto attr = new Attr(id);
  auto weak = v8::UniquePersistent<v8::Object>(v8::Isolate::GetCurrent(),
                                               AttributeWrapper::wrap(attr));
  info.GetReturnValue().Set(weak);
}
void createPayloadInstance(v8::FunctionCallbackInfo<v8::Value> const &info) {
  Token id = Token_get(*Nan::Utf8String(info[0]));
  info.GetReturnValue().Set(PayloadWrapper::wrap(new Payload()));
}
}
void PlugkitTesting::init(v8::Local<v8::Object> exports) {
  auto isolate = v8::Isolate::GetCurrent();
  auto testing = Nan::New<v8::Object>();
  exports->Set(Nan::New("Testing").ToLocalChecked(), testing);

  testing->Set(
      Nan::New("runCApiTests").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, runCApiTests, exports)->GetFunction());
  testing->Set(Nan::New("createAttrInstance").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, createAttrInstance, exports)
                   ->GetFunction());
  testing->Set(
      Nan::New("createPayloadInstance").ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, createPayloadInstance, exports)
          ->GetFunction());
}
}
