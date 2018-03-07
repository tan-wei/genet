#include "iterator.hpp"
#include "plugkit_module.hpp"
#include <vector>

namespace plugkit {

void IteratorWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Iterator").ToLocalChecked());
  SetPrototypeMethod(tpl, "next", next);
  PlugkitModule *module = PlugkitModule::get(isolate);
  module->iterator.ctor.Reset(isolate, Nan::GetFunction(tpl).ToLocalChecked());
}

IteratorWrapper::IteratorWrapper(const Callback cb, const void *data)
    : cb(cb), data(data) {}

NAN_METHOD(IteratorWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_METHOD(IteratorWrapper::next) {
  IteratorWrapper *wrapper = ObjectWrap::Unwrap<IteratorWrapper>(info.Holder());
  if (const auto &func = wrapper->cb) {
    auto value = func(wrapper->data, &wrapper->index, &wrapper->done);
    auto result = Nan::New<v8::Object>();
    result->Set(Nan::New("done").ToLocalChecked(), Nan::New(wrapper->done));
    result->Set(Nan::New("value").ToLocalChecked(), value);
    info.GetReturnValue().Set(result);
  }
}

v8::Local<v8::Object> IteratorWrapper::wrap(const Callback cb,
                                            const void *data) {
  auto ctx = Nan::New<v8::Object>();
  ctx->Set(Nan::New("cb").ToLocalChecked(),
           Nan::New<v8::External>(reinterpret_cast<void *>(cb)));
  ctx->Set(Nan::New("data").ToLocalChecked(),
           Nan::New<v8::External>(const_cast<void *>(data)));

  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  auto func = v8::FunctionTemplate::New(
      isolate,
      [](v8::FunctionCallbackInfo<v8::Value> const &info) {
        auto ctx = info.Data().As<v8::Object>();
        Callback cb =
            reinterpret_cast<Callback>(ctx->Get(Nan::New("cb").ToLocalChecked())
                                           .As<v8::External>()
                                           ->Value());
        const void *data = ctx->Get(Nan::New("data").ToLocalChecked())
                               .As<v8::External>()
                               ->Value();
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        PlugkitModule *module = PlugkitModule::get(isolate);
        auto cons =
            v8::Local<v8::Function>::New(isolate, module->iterator.ctor);
        v8::Local<v8::Object> obj =
            cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                              nullptr)
                .ToLocalChecked();
        IteratorWrapper *wrapper = new IteratorWrapper(cb, data);
        wrapper->Wrap(obj);
        info.GetReturnValue().Set(obj);
      },
      ctx);

  auto iterable = Nan::New<v8::Object>();
  iterable->Set(v8::Symbol::GetIterator(isolate), func->GetFunction());
  return iterable;
}

} // namespace plugkit
