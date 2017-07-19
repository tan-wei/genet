#include "wrapper/listener_status.hpp"
#include "wrapper/attribute.hpp"
#include "../listener_status.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void ListenerStatusWrapper::init(v8::Isolate *isolate,
                                 v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("ListenerStatus").ToLocalChecked());
  SetPrototypeMethod(tpl, "getAttribute", getAttribute);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("attributes").ToLocalChecked(), attributes);
  Nan::SetAccessor(otl, Nan::New("chunks").ToLocalChecked(), chunks);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->listenerStatus.proto.Reset(
      isolate, ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->listenerStatus.ctor.Reset(isolate, ctor);
  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("ListenerStatus").ToLocalChecked(), func);
}

ListenerStatusWrapper::ListenerStatusWrapper(
    const std::shared_ptr<ListenerStatus> &status)
    : status(status) {}

NAN_METHOD(ListenerStatusWrapper::New) {
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(ListenerStatusWrapper::attributes) {
  ListenerStatusWrapper *wrapper =
      ObjectWrap::Unwrap<ListenerStatusWrapper>(info.Holder());
  if (auto status = wrapper->status) {
    info.GetReturnValue().Set(static_cast<uint32_t>(status->attributes()));
  }
}

NAN_GETTER(ListenerStatusWrapper::chunks) {
  ListenerStatusWrapper *wrapper =
      ObjectWrap::Unwrap<ListenerStatusWrapper>(info.Holder());
  if (auto status = wrapper->status) {
    info.GetReturnValue().Set(static_cast<uint32_t>(status->chunks()));
  }
}

NAN_METHOD(ListenerStatusWrapper::getAttribute) {
  ListenerStatusWrapper *wrapper =
      ObjectWrap::Unwrap<ListenerStatusWrapper>(info.Holder());
  if (auto status = wrapper->status) {
    if (const auto &attr = status->getAttribute(info[0]->NumberValue())) {
      info.GetReturnValue().Set(AttributeWrapper::wrap(attr));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

v8::Local<v8::Object>
ListenerStatusWrapper::wrap(const std::shared_ptr<ListenerStatus> &status) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons =
      v8::Local<v8::Function>::New(isolate, module->listenerStatus.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  ListenerStatusWrapper *wrapper = new ListenerStatusWrapper(status);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<ListenerStatus>
ListenerStatusWrapper::unwrap(v8::Local<v8::Object> obj) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  if (obj->GetPrototype() ==
      v8::Local<v8::Value>::New(isolate, module->listenerStatus.proto)) {
    if (auto wrapper = ObjectWrap::Unwrap<ListenerStatusWrapper>(obj)) {
      return wrapper->status;
    }
  }
  return nullptr;
}
}
