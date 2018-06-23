#include "attr.hpp"
#include "exports.hpp"
#include "layer.hpp"
#include "module.hpp"

namespace genet_node {

void AttrWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Attr").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "get", get);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
  Nan::SetAccessor(otl, Nan::New("type").ToLocalChecked(), type);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_ATTR);
  cls.ctor.Reset(isolate, ctor);
}

NAN_METHOD(AttrWrapper::New) {
  if (info.IsConstructCall()) {
    auto attr = info[0];
    if (attr->IsExternal()) {
      auto obj = static_cast<AttrWrapper *>(attr.As<v8::External>()->Value());
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
  }
}

AttrWrapper::AttrWrapper(const Attr *attr) : attr(attr) {}

AttrWrapper::~AttrWrapper() {}

NAN_GETTER(AttrWrapper::id) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    info.GetReturnValue().Set(genet_attr_id(attr));
  }
}

NAN_GETTER(AttrWrapper::type) {
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    info.GetReturnValue().Set(genet_attr_type(attr));
  }
}

NAN_METHOD(AttrWrapper::get) {
  const Layer *layer = LayerWrapper::unwrap(info[0]).getConst();
  if (!layer) {
    Nan::ThrowTypeError("First argument must be a Layer");
    return;
  }
  AttrWrapper *wrapper = ObjectWrap::Unwrap<AttrWrapper>(info.Holder());
  if (auto attr = wrapper->attr) {
    const Variant &var = genet_attr_get(attr, layer);
    v8::Local<v8::Value> val = Nan::Null();
    switch (var.type) {
    case Error:
      Nan::ThrowError(Nan::New(var.data, var.value.u64).ToLocalChecked());
      genet_str_free(var.data);
      return;
    case Bool:
      val = Nan::New(var.value.u64 != 0);
      break;
    case Int64:
      val = Nan::New(static_cast<int32_t>(var.value.i64));
      break;
    case UInt64:
      val = Nan::New(static_cast<uint32_t>(var.value.u64));
      break;
    case Float64:
      val = Nan::New(var.value.f64);
      break;
    case String:
      val = Nan::New(var.data, var.value.u64).ToLocalChecked();
      genet_str_free(var.data);
      break;
    case Buffer:
      val = Nan::CopyBuffer(var.data, var.value.u64).ToLocalChecked();
      genet_str_free(var.data);
      break;
    case Slice:
      val = Nan::NewBuffer(var.data, var.value.u64,
                           [](char *data, void *hint) {}, nullptr)
                .ToLocalChecked();
      break;
    }
    info.GetReturnValue().Set(val);
  }
}

v8::Local<v8::Object> AttrWrapper::wrap(const Attr *attr) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  const auto &cls = Module::current().get(Module::CLASS_ATTR);
  auto cons = v8::Local<v8::Function>::New(isolate, cls.ctor);
  auto ptr = new AttrWrapper(attr);
  v8::Local<v8::Value> args[] = {Nan::New<v8::External>(ptr)};
  return Nan::NewInstance(cons, 1, args).ToLocalChecked();
}

} // namespace genet_node
