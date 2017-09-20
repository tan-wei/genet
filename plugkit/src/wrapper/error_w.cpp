#include "error.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

ErrorWrapper::ErrorWrapper(const Error &error) : error(error) {}

void ErrorWrapper::init(v8::Isolate *isolate, v8::Local<v8::Object> exports) {

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Error").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("target").ToLocalChecked(), target);
  Nan::SetAccessor(otl, Nan::New("type").ToLocalChecked(), type);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("Error").ToLocalChecked(), func);
}

NAN_METHOD(ErrorWrapper::New) {
  Token target = info[0]->IsNumber() ? info[0]->NumberValue()
                                     : Token_get(*Nan::Utf8String(info[0]));
  Token type = info[1]->IsNumber() ? info[1]->NumberValue()
                                   : Token_get(*Nan::Utf8String(info[1]));
  ErrorWrapper *obj = new ErrorWrapper(Error{target, type});
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ErrorWrapper::target) {
  ErrorWrapper *wrapper = ObjectWrap::Unwrap<ErrorWrapper>(info.Holder());
  info.GetReturnValue().Set(
      Nan::New(Token_string(wrapper->error.target)).ToLocalChecked());
}

NAN_GETTER(ErrorWrapper::type) {
  ErrorWrapper *wrapper = ObjectWrap::Unwrap<ErrorWrapper>(info.Holder());
  info.GetReturnValue().Set(
      Nan::New(Token_string(wrapper->error.type)).ToLocalChecked());
}

v8::Local<v8::Object> ErrorWrapper::wrap(const Error &error) {
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

Error ErrorWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->error.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ErrorWrapper>(obj)) {
      return wrapper->error;
    }
  }
  return Error{};
}
}
