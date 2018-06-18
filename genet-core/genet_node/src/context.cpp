#include "context.hpp"
#include "exports.hpp"
#include "module.hpp"

namespace genet_node {

void ContextWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Context").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "token", token);
  Nan::SetPrototypeMethod(tpl, "string", string);
  Nan::SetPrototypeMethod(tpl, "closeStream", closeStream);
  Nan::SetPrototypeMethod(tpl, "getConfig", getConfig);

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto& cls = Module::current().get(Module::CLASS_CONTEXT);
  cls.ctor.Reset(isolate, ctor);
}

NAN_METHOD(ContextWrapper::New) {
  if (info.IsConstructCall()) {
    auto ctx = info[0];
    if (ctx->IsExternal()) {
      auto obj = static_cast<ContextWrapper*>(ctx.As<v8::External>()->Value());
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }
}

NAN_METHOD(ContextWrapper::token) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Nan::Utf8String id(info[0]);
  if (auto wrapper = Nan::ObjectWrap::Unwrap<ContextWrapper>(info.Holder())) {
    info.GetReturnValue().Set(genet_token_get(*id));
  }
}

NAN_METHOD(ContextWrapper::string) {
  if (!info[0]->IsUint32()) {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  uint32_t token = info[0]->Uint32Value();
  if (auto wrapper = Nan::ObjectWrap::Unwrap<ContextWrapper>(info.Holder())) {
    char* str = genet_token_string(token);
    info.GetReturnValue().Set(Nan::New(str).ToLocalChecked());
    genet_str_free(str);
  }
}

NAN_METHOD(ContextWrapper::closeStream) {
  if (auto wrapper = Nan::ObjectWrap::Unwrap<ContextWrapper>(info.Holder())) {
    genet_context_close_stream(wrapper->ctx.get());
  }
}

NAN_METHOD(ContextWrapper::getConfig) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Nan::Utf8String key(info[0]);
  if (auto wrapper = Nan::ObjectWrap::Unwrap<ContextWrapper>(info.Holder())) {
    char* json = genet_context_get_config(wrapper->ctx.get(), *key);
    v8::Local<v8::String> json_string = Nan::New(json).ToLocalChecked();
    genet_str_free(json);
    Nan::JSON NanJSON;
    Nan::MaybeLocal<v8::Value> result = NanJSON.Parse(json_string);
    if (!result.IsEmpty()) {
      info.GetReturnValue().Set(result.ToLocalChecked());
    }
  }
}

ContextWrapper::ContextWrapper(const Pointer<Context>& ctx) : ctx(ctx) {}

ContextWrapper::~ContextWrapper() {
  genet_context_free(ctx.getOwned());
}

v8::Local<v8::Object> ContextWrapper::wrap(const Pointer<Context>& ctx) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  const auto& cls = Module::current().get(Module::CLASS_CONTEXT);
  auto cons = v8::Local<v8::Function>::New(isolate, cls.ctor);
  auto ptr = new ContextWrapper(ctx);
  v8::Local<v8::Value> args[] = {Nan::New<v8::External>(ptr)};
  return Nan::NewInstance(cons, 1, args).ToLocalChecked();
}

}  // namespace genet_node