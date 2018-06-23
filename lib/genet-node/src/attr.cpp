#include "attr.hpp"
#include "exports.hpp"
#include "module.hpp"

namespace genet_node {

void AttrWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Attr").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_ATTR);
  cls.ctor.Reset(isolate, ctor);
}

NAN_METHOD(AttrWrapper::New) {
  if (info.IsConstructCall()) {
    auto frame = info[0];
    if (frame->IsExternal()) {
      auto obj = static_cast<AttrWrapper *>(frame.As<v8::External>()->Value());
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }
}

AttrWrapper::AttrWrapper(const Attr *frame) : frame(frame) {}

AttrWrapper::~AttrWrapper() {}

v8::Local<v8::Object> AttrWrapper::wrap(const Attr *frame) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  const auto &cls = Module::current().get(Module::CLASS_ATTR);
  auto cons = v8::Local<v8::Function>::New(isolate, cls.ctor);
  auto ptr = new AttrWrapper(frame);
  v8::Local<v8::Value> args[] = {Nan::New<v8::External>(ptr)};
  return Nan::NewInstance(cons, 1, args).ToLocalChecked();
}

} // namespace genet_node
