#include "plugkit_module.hpp"
#include "embedded_files.hpp"
#include "extended_slot.hpp"
#include "plugkit_testing.hpp"
#include "token.h"
#include "variant.hpp"
#include "wrapper/attr.hpp"
#include "wrapper/context.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/logger.hpp"
#include "wrapper/payload.hpp"
#include "wrapper/pcap.hpp"
#include "wrapper/session.hpp"
#include "wrapper/session_factory.hpp"

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

NAN_METHOD(Token_join_wrap) {

  Token prefix = Token_get(nullptr);
  auto prefixId = info[0];
  if (prefixId->IsUint32()) {
    prefix = prefixId->Uint32Value();
  } else if (prefixId->IsString()) {
    prefix = Token_get(*Nan::Utf8String(prefixId));
  } else {
    Nan::ThrowTypeError("First argument must be a string or token-id");
    return;
  }

  Token token = Token_get(nullptr);
  auto tokenId = info[1];
  if (tokenId->IsUint32()) {
    token = tokenId->Uint32Value();
  } else if (tokenId->IsString()) {
    token = Token_get(*Nan::Utf8String(tokenId));
  } else {
    Nan::ThrowTypeError("Second argument must be a string or token-id");
    return;
  }

  Token joined = Token_join(prefix, token);
  info.GetReturnValue().Set(joined);
}

NAN_METHOD(Token_string_wrap) {
  if (!info[0]->IsUint32()) {
    Nan::ThrowTypeError("First argument must be a number");
    return;
  }
  auto str = Nan::New(Token_string(info[0]->Uint32Value())).ToLocalChecked();
  info.GetReturnValue().Set(str);
}
} // namespace

PlugkitModule::PlugkitModule(v8::Isolate *isolate,
                             v8::Local<v8::Object> exports,
                             bool mainThread) {
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE, this);
  Variant::init(isolate);
  AttributeWrapper::init(isolate, exports);
  PayloadWrapper::init(isolate, exports);
  LayerWrapper::init(isolate);
  FrameWrapper::init(isolate);
  ContextWrapper::init(isolate);
  LoggerWrapper::init(isolate);
  if (mainThread) {
    PcapWrapper::init(isolate, exports);
    SessionFactoryWrapper::init(isolate, exports);
    SessionWrapper::init(isolate);
  }

  auto token = Nan::New<v8::Object>();
  token->Set(Nan::New("get").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_get_wrap)->GetFunction());
  token->Set(Nan::New("join").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_join_wrap)->GetFunction());
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

  PlugkitTesting::init(exports);
}

PlugkitModule *PlugkitModule::get(v8::Isolate *isolate) {
  return ExtendedSlot::get<PlugkitModule>(isolate,
                                          ExtendedSlot::SLOT_PLUGKIT_MODULE);
}
} // namespace plugkit
