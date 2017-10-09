#include "plugkit_module.hpp"
#include "extended_slot.hpp"
#include "plugkit_testing.hpp"
#include "token.h"
#include "variant.hpp"
#include "wrapper/attribute.hpp"
#include "wrapper/context.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/logger.hpp"
#include "wrapper/payload.hpp"
#include "wrapper/pcap.hpp"
#include "wrapper/session.hpp"
#include "wrapper/session_factory.hpp"
#include "wrapper/worker.hpp"
#include "embedded_files.hpp"

namespace plugkit {

namespace {
NAN_METHOD(Token_get_wrap) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Token token = Token_get(*Nan::Utf8String(info[0]));
  info.GetReturnValue().Set(token);
}
NAN_METHOD(Token_string_wrap) {
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("First argument must be a number");
    return;
  }
  auto str = Nan::New(Token_string(info[0]->NumberValue())).ToLocalChecked();
  info.GetReturnValue().Set(str);
}
} // namespace

PlugkitModule::PlugkitModule(v8::Isolate *isolate,
                             v8::Local<v8::Object> exports, bool mainThread) {
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE, this);
  Variant::init(isolate);
  AttributeWrapper::init(isolate);
  LayerWrapper::init(isolate);
  FrameWrapper::init(isolate);
  ContextWrapper::init(isolate);
  LoggerWrapper::init(isolate);
  PayloadWrapper::init(isolate);
  WorkerWrapper::init(isolate, exports);
  if (mainThread) {
    PcapWrapper::init(isolate, exports);
    SessionFactoryWrapper::init(isolate, exports);
    SessionWrapper::init(isolate);
  }

  auto token = Nan::New<v8::Object>();
  token->Set(Nan::New("get").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_get_wrap)->GetFunction());
  token->Set(Nan::New("string").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_string_wrap)->GetFunction());
  exports->Set(Nan::New("Token").ToLocalChecked(), token);

  for (const char *data : files) {
    auto script = Nan::CompileScript(Nan::New(data).ToLocalChecked());
    auto func = Nan::RunScript(script.ToLocalChecked())
                    .ToLocalChecked()
                    .As<v8::Function>();
    auto global = isolate->GetCurrentContext()->Global();
    v8::Local<v8::Value> args[1] = {exports};
    func->Call(global, 1, args);
  }

#ifdef PLUGKIT_ENABLE_TESTING
  PlugkitTesting::init(exports);
#endif
}

PlugkitModule *PlugkitModule::get(v8::Isolate *isolate) {
  return ExtendedSlot::get<PlugkitModule>(isolate,
                                          ExtendedSlot::SLOT_PLUGKIT_MODULE);
}
} // namespace plugkit
