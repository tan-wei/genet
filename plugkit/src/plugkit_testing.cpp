#include "plugkit_testing.hpp"
#include <nan.h>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

namespace plugkit {
void PlugkitTesting::init(v8::Local<v8::Object> exports) {
  auto isolate = v8::Isolate::GetCurrent();
  auto testing = Nan::New<v8::Object>();
  exports->Set(Nan::New("Testing").ToLocalChecked(), testing);

  auto runTests = v8::FunctionTemplate::New(
      isolate, [](v8::FunctionCallbackInfo<v8::Value> const &info) {
        int result = Catch::Session().run();
        info.GetReturnValue().Set(result);
      }, exports);

  testing->Set(Nan::New("runCApiTests").ToLocalChecked(),
               runTests->GetFunction());
}
}
