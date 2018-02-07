#include "../error.hpp"
#include "error.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void ErrorWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Error").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("target").ToLocalChecked(), target, setTarget);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->error.proto.Reset(isolate,
                            ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->error.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Error").ToLocalChecked(), ctor);
}

ErrorWrapper::ErrorWrapper(Error *error) : error(error), constError(error) {}

ErrorWrapper::ErrorWrapper(const Error *error)
    : error(nullptr), constError(error) {}

NAN_METHOD(ErrorWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(ErrorWrapper::id) {
  ErrorWrapper *wrapper = ObjectWrap::Unwrap<ErrorWrapper>(info.Holder());
  if (auto error = wrapper->constError) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(error->id)).ToLocalChecked());
  }
}

NAN_GETTER(ErrorWrapper::target) {
  ErrorWrapper *wrapper = ObjectWrap::Unwrap<ErrorWrapper>(info.Holder());
  if (auto error = wrapper->constError) {
    info.GetReturnValue().Set(
        Nan::New(Token_string(error->target)).ToLocalChecked());
  }
}

NAN_SETTER(ErrorWrapper::setTarget) {
  ErrorWrapper *wrapper = ObjectWrap::Unwrap<ErrorWrapper>(info.Holder());
  if (auto error = wrapper->error) {
    Token token = value->IsUint32() ? value->Uint32Value()
                                    : Token_get(*Nan::Utf8String(value));
    error->target = token;
  }
}

v8::Local<v8::Object> ErrorWrapper::wrap(Error *error) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->error.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ErrorWrapper *wrapper = new ErrorWrapper(error);
  wrapper->Wrap(obj);
  return obj;
}

v8::Local<v8::Object> ErrorWrapper::wrap(const Error *error) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->error.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ErrorWrapper *wrapper = new ErrorWrapper(error);
  wrapper->Wrap(obj);
  return obj;
}

const Error *ErrorWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  auto obj = value.As<v8::Object>();
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->error.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ErrorWrapper>(obj)) {
      return wrapper->constError;
    }
  }
  return nullptr;
}
} // namespace plugkit
