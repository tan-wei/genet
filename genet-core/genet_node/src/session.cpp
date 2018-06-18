#include "session.hpp"
#include "context.hpp"
#include "genet_kernel.hpp"
#include "frame.hpp"
#include "module.hpp"
#include "script.hpp"
#include <vector>

namespace genet_node {

namespace {
int marker = 0;
}

void SessionProfileWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(2);
  tpl->SetClassName(Nan::New("Profile").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "setConfig", setConfig);
  Nan::SetPrototypeMethod(tpl, "addLinkLayer", addLinkLayer);

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto& cls = Module::current().get(Module::CLASS_SESSION_PROFILE);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("SessionProfile").ToLocalChecked(), ctor);
}

NAN_METHOD(SessionProfileWrapper::New) {
  if (info.IsConstructCall()) {
    SessionProfileWrapper* obj = new SessionProfileWrapper(
        Pointer<SessionProfile>::owned(genet_session_profile_new()));
    obj->Wrap(info.This());
    Nan::SetInternalFieldPointer(info.This(), 1, &marker);
    info.GetReturnValue().Set(info.This());
  }
}

NAN_METHOD(SessionProfileWrapper::setConfig) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Nan::JSON NanJSON;
  auto result = NanJSON.Stringify(info[1].As<v8::Object>());
  if (!result.IsEmpty()) {
    Nan::Utf8String key(info[0]);
    Nan::Utf8String value(result.ToLocalChecked());
    if (auto wrapper =
            Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
      genet_session_profile_set_config(wrapper->profile.get(), *key, *value);
    }
  }
}

NAN_METHOD(SessionProfileWrapper::addLinkLayer) {
  if (!info[0]->IsInt32()) {
    Nan::ThrowTypeError("First argument must be an object");
    return;
  }
  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Second argument must be a string");
    return;
  }
  if (auto wrapper =
          Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
    int32_t link = info[0]->Int32Value();
    Nan::Utf8String id(info[1]);
    genet_session_profile_add_link_layer(wrapper->profile.get(), link, *id);
  }
}

SessionProfileWrapper::SessionProfileWrapper(
    const Pointer<SessionProfile>& profile)
    : profile(profile) {}

SessionProfileWrapper::~SessionProfileWrapper() {
  genet_session_profile_free(profile.getOwned());
}

Pointer<SessionProfile> SessionProfileWrapper::unwrap(
    v8::Local<v8::Value> value) {
  if (!value.IsEmpty() && value->IsObject()) {
    auto object = value.As<v8::Object>();
    if (Nan::GetInternalFieldPointer(object, 1) == &marker) {
      if (auto wrapper =
              Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(object)) {
        return wrapper->profile;
      }
    }
  }
  return Pointer<SessionProfile>::null();
}

void SessionWrapper::init(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(2);
  tpl->SetClassName(Nan::New("Session").ToLocalChecked());
  Nan::SetPrototypeMethod(tpl, "close", close);
  Nan::SetPrototypeMethod(tpl, "pushFrames", pushFrames);
  Nan::SetPrototypeMethod(tpl, "frames", frames);
  Nan::SetPrototypeMethod(tpl, "filteredFrames", filteredFrames);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("callback").ToLocalChecked(), callback,
                   setCallback);
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);
  Nan::SetAccessor(otl, Nan::New("context").ToLocalChecked(), context);

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto& cls = Module::current().get(Module::CLASS_SESSION);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Session").ToLocalChecked(), ctor);
}

NAN_METHOD(SessionWrapper::New) {
  if (info.IsConstructCall()) {
    if (auto profile = SessionProfileWrapper::unwrap(info[0]).getConst()) {
      SessionWrapper* obj = new SessionWrapper();
      obj->event->session =
          genet_session_new(profile,
                           [](void* data, char* event) {
                             auto ev = static_cast<Event*>(data);
                             {
                               std::lock_guard<std::mutex> lock(ev->mutex);
                               ev->queue.push_back(event);
                             }
                             uv_async_send(&ev->async);
                           },
                           obj->event);
      obj->Wrap(info.This());
      Nan::SetInternalFieldPointer(info.This(), 1, &marker);
      info.GetReturnValue().Set(info.This());
    } else {
      Nan::ThrowTypeError("First argument must be a profile");
    }
  }
}

NAN_GETTER(SessionWrapper::context) {
  SessionWrapper* wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (!wrapper->event) {
    Nan::ThrowReferenceError("Session has been closed");
    return;
  }
  Session* session = wrapper->event->session;
  Context* ctx = genet_session_context(session);
  info.GetReturnValue().Set(ContextWrapper::wrap(Pointer<Context>::owned(ctx)));
}

NAN_METHOD(SessionWrapper::pushFrames) {
  if (!info[0]->IsArray()) {
    Nan::ThrowTypeError("First argument must be an array");
    return;
  }
  if (!info[1]->IsInt32()) {
    Nan::ThrowTypeError("Second argument must be an integer");
    return;
  }
  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    auto array = info[0].As<v8::Array>();
    int32_t link = info[1]->Int32Value();
    Session* session = wrapper->event->session;
    for (uint32_t i = 0; i < array->Length(); ++i) {
      auto value = array->Get(i);
      if (value->IsObject()) {
        auto obj = value.As<v8::Object>();
        auto data = obj->Get(Nan::New("data").ToLocalChecked());
        if (data->IsArrayBufferView()) {
          auto view = data.As<v8::ArrayBufferView>();
          size_t len = view->ByteLength();
          char* buf = static_cast<char*>(view->Buffer()->GetContents().Data()) +
                      view->ByteOffset();
          genet_session_push_frame(session, buf, len, link);
        }
      }
    }
  }
}

NAN_METHOD(SessionWrapper::frames) {
  uint32_t start = 0;
  uint32_t end = 0;
  if (info[0]->IsUint32()) {
    start = info[0]->Uint32Value();
  } else {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  if (info[1]->IsUint32()) {
    end = info[1]->Uint32Value();
  } else {
    Nan::ThrowTypeError("Second argument must be an integer");
    return;
  }

  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session* session = wrapper->event->session;

    size_t length = end - start;
    std::vector<const Frame*> dst;
    dst.resize(length);
    genet_session_frames(session, start, end, &length, dst.data());
    dst.resize(length);

    auto array = Nan::New<v8::Array>(length);
    for (uint32_t index = 0; index < length; ++index) {
      array->Set(index, FrameWrapper::wrap(dst[index]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(SessionWrapper::filteredFrames) {
  uint32_t id = 0;
  uint32_t start = 0;
  uint32_t end = 0;
  if (info[0]->IsUint32()) {
    id = info[0]->Uint32Value();
  } else {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  if (info[1]->IsUint32()) {
    start = info[1]->Uint32Value();
  } else {
    Nan::ThrowTypeError("Second argument must be an integer");
    return;
  }
  if (info[2]->IsUint32()) {
    end = info[2]->Uint32Value();
  } else {
    Nan::ThrowTypeError("Third argument must be an integer");
    return;
  }

  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session* session = wrapper->event->session;

    size_t length = end - start;
    std::vector<const Frame*> dst;
    dst.resize(length);
    genet_session_filtered_frames(session, id, start, end, &length, dst.data());
    dst.resize(length);

    auto array = Nan::New<v8::Array>(length);
    for (uint32_t index = 0; index < length; ++index) {
      array->Set(index, FrameWrapper::wrap(dst[index]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_GETTER(SessionWrapper::callback) {
  SessionWrapper* wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (!wrapper->event->callback.IsEmpty()) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    auto func = v8::Local<v8::Function>::New(isolate, wrapper->event->callback);
    info.GetReturnValue().Set(func);
  }
}

NAN_SETTER(SessionWrapper::setCallback) {
  SessionWrapper* wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (value->IsFunction()) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    wrapper->event->callback.Reset(isolate, value.As<v8::Function>());
  }
}

NAN_GETTER(SessionWrapper::length) {
  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session* session = wrapper->event->session;
    info.GetReturnValue().Set(genet_session_len(session));
  }
}

SessionWrapper::SessionWrapper() : event(new Event()) {
  event->async.data = event;
  uv_async_init(uv_default_loop(), &event->async, [](uv_async_t* handle) {
    Event* event = reinterpret_cast<Event*>(handle->data);
    std::lock_guard<std::mutex> lock(event->mutex);

    if (!event->callback.IsEmpty()) {
      v8::Isolate* isolate = v8::Isolate::GetCurrent();
      v8::HandleScope handle_scope(isolate);
      auto global = isolate->GetCurrentContext()->Global();
      auto func = v8::Local<v8::Function>::New(isolate, event->callback);

      for (char* json : event->queue) {
        v8::Local<v8::String> json_string = Nan::New(json).ToLocalChecked();
        genet_str_free(json);
        Nan::JSON NanJSON;
        Nan::MaybeLocal<v8::Value> result = NanJSON.Parse(json_string);
        if (!result.IsEmpty()) {
          v8::Local<v8::Value> args[1] = {result.ToLocalChecked()};
          func->Call(global, 1, args);
        }
      }
    }

    event->queue.clear();
  });
}

void SessionWrapper::handleEvent(const char* event) {}

void SessionWrapper::close() {
  if (event) {
    genet_session_free(event->session);
    uv_close(reinterpret_cast<uv_handle_t*>(&event->async),
             [](uv_handle_t* handle) {
               Event* event = reinterpret_cast<Event*>(handle->data);
               delete event;
             });
    event = nullptr;
  }
}

SessionWrapper::~SessionWrapper() {}

NAN_METHOD(SessionWrapper::close) {
  SessionWrapper* wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  wrapper->close();
}

}  // namespace genet_node
