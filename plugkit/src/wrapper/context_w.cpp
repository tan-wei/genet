#include "../context.hpp"
#include "context.hpp"
#include "layer.hpp"
#include "plugkit_module.hpp"
#include "variant.hpp"
#include <vector>

namespace plugkit {

void ContextWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Context").ToLocalChecked());
  SetPrototypeMethod(tpl, "getConfig", getConfig);
  SetPrototypeMethod(tpl, "closeStream", closeStream);
  SetPrototypeMethod(tpl, "addLayerLinkage", addLayerLinkage);

  PlugkitModule *module = PlugkitModule::get(isolate);
  module->context.ctor.Reset(isolate, Nan::GetFunction(tpl).ToLocalChecked());
}

ContextWrapper::ContextWrapper(Context *ctx) : ctx(ctx) {}

NAN_METHOD(ContextWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_METHOD(ContextWrapper::getConfig) {
  ContextWrapper *wrapper = ObjectWrap::Unwrap<ContextWrapper>(info.Holder());
  if (Context *ctx = wrapper->ctx) {
    const auto &str = Nan::Utf8String(info[0]);
    info.GetReturnValue().Set(
        Variant::getValue(Context_getConfig(ctx, *str, str.length())));
  }
}

NAN_METHOD(ContextWrapper::closeStream) {
  ContextWrapper *wrapper = ObjectWrap::Unwrap<ContextWrapper>(info.Holder());
  if (Context *ctx = wrapper->ctx) {
    Context_closeStream(ctx);
  }
}

NAN_METHOD(ContextWrapper::addLayerLinkage) {
  ContextWrapper *wrapper = ObjectWrap::Unwrap<ContextWrapper>(info.Holder());
  if (Context *ctx = wrapper->ctx) {
    Token token = Token_null();
    auto tok = info[0];
    if (tok->IsUint32()) {
      token = tok->Uint32Value();
    } else if (tok->IsString()) {
      token = Token_get(*Nan::Utf8String(tok));
    } else {
      Nan::ThrowTypeError("First argument must be a string or token-id");
      return;
    }
    if (!info[1]->IsUint32()) {
      Nan::ThrowTypeError("Second argument must be an integer");
      return;
    }
    if (Layer *layer = LayerWrapper::unwrap(info[2])) {
      Context_addLayerLinkage(ctx, token, info[1]->Uint32Value(), layer);
    } else {
      Nan::ThrowTypeError("Third argument must be a Layer");
    }
  }
}

v8::Local<v8::Object> ContextWrapper::wrap(Context *ctx) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->context.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ContextWrapper *wrapper = new ContextWrapper(ctx);
  wrapper->Wrap(obj);
  return obj;
}

Context *ContextWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  if (auto wrapper =
          ObjectWrap::Unwrap<ContextWrapper>(value.As<v8::Object>())) {
    return wrapper->ctx;
  }
  return nullptr;
}
} // namespace plugkit
