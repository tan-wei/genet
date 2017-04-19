#include "frame.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "private/variant.hpp"
#include "../plugkit/frame.hpp"
#include "plugkit_module.hpp"
#include "extended_slot.hpp"

namespace plugkit {

void FrameWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Frame").ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("timestamp").ToLocalChecked(), timestamp);
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);
  Nan::SetAccessor(otl, Nan::New("seq").ToLocalChecked(), seq);
  Nan::SetAccessor(otl, Nan::New("rootLayer").ToLocalChecked(), rootLayer);
  Nan::SetAccessor(otl, Nan::New("primaryLayer").ToLocalChecked(),
                   primaryLayer);
  Nan::SetAccessor(otl, Nan::New("leafLayers").ToLocalChecked(), leafLayers);
  Nan::SetAccessor(otl, Nan::New("hasError").ToLocalChecked(), hasError);

  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  module->frame.ctor.Reset(isolate, Nan::GetFunction(tpl).ToLocalChecked());
}

FrameWrapper::FrameWrapper(const std::weak_ptr<const FrameView> &view)
    : view(view) {}

NAN_METHOD(FrameWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(FrameWrapper::timestamp) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    view->frame()->timestamp().time_since_epoch())
                    .count();
    info.GetReturnValue().Set(v8::Date::New(isolate, nano / 1000000.0));
  }
}

NAN_GETTER(FrameWrapper::length) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    info.GetReturnValue().Set(static_cast<uint32_t>(view->frame()->length()));
  }
}

NAN_GETTER(FrameWrapper::seq) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    info.GetReturnValue().Set(view->frame()->seq());
  }
}

NAN_GETTER(FrameWrapper::rootLayer) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    if (const auto &layer = view->frame()->rootLayer()) {
      info.GetReturnValue().Set(LayerWrapper::wrap(layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_GETTER(FrameWrapper::primaryLayer) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    if (const auto &layer = view->primaryLayer()) {
      info.GetReturnValue().Set(LayerWrapper::wrap(layer));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

NAN_GETTER(FrameWrapper::leafLayers) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const auto &layers = view->leafLayers();
    auto array = v8::Array::New(isolate, layers.size());
    for (size_t i = 0; i < layers.size(); ++i) {
      array->Set(i, LayerWrapper::wrap(layers[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(FrameWrapper::hasError) {
  FrameWrapper *wrapper = ObjectWrap::Unwrap<FrameWrapper>(info.Holder());
  if (const auto &view = wrapper->view.lock()) {
    info.GetReturnValue().Set(view->hasError());
  }
}

v8::Local<v8::Object>
FrameWrapper::wrap(const std::weak_ptr<const FrameView> &view) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto cons = v8::Local<v8::Function>::New(isolate, module->frame.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  FrameWrapper *wrapper = new FrameWrapper(view);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<const FrameView>
FrameWrapper::unwrap(v8::Local<v8::Object> obj) {
  if (auto wrapper = ObjectWrap::Unwrap<FrameWrapper>(obj)) {
    return wrapper->view.lock();
  }
  return std::shared_ptr<FrameView>();
}
}
