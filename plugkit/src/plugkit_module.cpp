#include "plugkit_module.hpp"
#include "extended_slot.hpp"
#include "token.h"
#include "variant.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/pcap.hpp"
#include "wrapper/payload.hpp"
#include "wrapper/property.hpp"
#include "wrapper/session.hpp"
#include "wrapper/session_factory.hpp"

namespace plugkit {

namespace {
NAN_METHOD(Token_get_wrap) {
  Token token = Token_get(*Nan::Utf8String(info[0]));
  info.GetReturnValue().Set(token);
}
NAN_METHOD(Token_string_wrap) {
  auto str = Nan::New(Token_string(info[0]->NumberValue())).ToLocalChecked();
  info.GetReturnValue().Set(str);
}
}

PlugkitModule::PlugkitModule(v8::Isolate *isolate,
                             v8::Local<v8::Object> exports, bool mainThread) {
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE, this);
  Variant::init(isolate);
  PropertyWrapper::init(isolate, exports);
  LayerWrapper::init(isolate, exports);
  FrameWrapper::init(isolate);
  PayloadWrapper::init(isolate);
  if (mainThread) {
    PcapWrapper::init(isolate, exports);
    SessionFactoryWrapper::init(isolate, exports);
    SessionWrapper::init(isolate, exports);
  }

  auto token = Nan::New<v8::Object>();
  token->Set(Nan::New("get").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_get_wrap)->GetFunction());
  token->Set(Nan::New("string").ToLocalChecked(),
             Nan::New<v8::FunctionTemplate>(Token_string_wrap)->GetFunction());
  exports->Set(Nan::New("Token").ToLocalChecked(), token);
}

PlugkitModule *PlugkitModule::get(v8::Isolate *isolate) {
  return ExtendedSlot::get<PlugkitModule>(isolate,
                                          ExtendedSlot::SLOT_PLUGKIT_MODULE);
}
}
