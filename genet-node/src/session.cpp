#include "session.hpp"
#include "exports.hpp"
#include "filter.hpp"
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
  Nan::SetPrototypeMethod(tpl, "loadLibrary", loadLibrary);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("concurrency").ToLocalChecked(), concurrency,
                   setConcurrency);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_SESSION_PROFILE);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("SessionProfile").ToLocalChecked(), ctor);
}

NAN_METHOD(SessionProfileWrapper::New) {
  if (info.IsConstructCall()) {
    SessionProfileWrapper *obj = new SessionProfileWrapper(
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
  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Second argument must be a string");
    return;
  }
  Nan::Utf8String key(info[0]);
  Nan::Utf8String value(info[1]);
  if (auto wrapper =
          Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
    genet_session_profile_set_config(wrapper->profile.get(), *key, *value);
  }
}

NAN_METHOD(SessionProfileWrapper::loadLibrary) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  Nan::Utf8String path(info[0]);
  if (auto wrapper =
          Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
    if (char *error =
            genet_session_profile_load_library(wrapper->profile.get(), *path)) {
      Nan::ThrowError(Nan::New(error).ToLocalChecked());
      genet_str_free(error);
    }
  }
}

NAN_GETTER(SessionProfileWrapper::concurrency) {
  if (auto wrapper =
          Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
    info.GetReturnValue().Set(
        genet_session_profile_concurrency(wrapper->profile.getConst()));
  }
}

NAN_SETTER(SessionProfileWrapper::setConcurrency) {
  if (!value->IsUint32()) {
    Nan::ThrowTypeError("Value must be an integer");
    return;
  }
  if (auto wrapper =
          Nan::ObjectWrap::Unwrap<SessionProfileWrapper>(info.Holder())) {
    genet_session_profile_set_concurrency(wrapper->profile.get(),
                                          value->Uint32Value());
  }
}

SessionProfileWrapper::SessionProfileWrapper(
    const Pointer<SessionProfile> &profile)
    : profile(profile) {}

SessionProfileWrapper::~SessionProfileWrapper() {
  genet_session_profile_free(profile.getOwned());
}

Pointer<SessionProfile>
SessionProfileWrapper::unwrap(v8::Local<v8::Value> value) {
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
  Nan::SetPrototypeMethod(tpl, "frames", frames);
  Nan::SetPrototypeMethod(tpl, "filteredFrames", filteredFrames);
  Nan::SetPrototypeMethod(tpl, "setFilter", setFilter);
  Nan::SetPrototypeMethod(tpl, "createReader", createReader);
  Nan::SetPrototypeMethod(tpl, "createWriter", createWriter);
  Nan::SetPrototypeMethod(tpl, "closeReader", closeReader);
  Nan::SetPrototypeMethod(tpl, "closeWriter", closeWriter);
  Nan::SetPrototypeMethod(tpl, "getMetadata", getMetadata);

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("callback").ToLocalChecked(), callback,
                   setCallback);
  Nan::SetAccessor(otl, Nan::New("length").ToLocalChecked(), length);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  auto &cls = Module::current().get(Module::CLASS_SESSION);
  cls.ctor.Reset(isolate, ctor);
  Nan::Set(exports, Nan::New("Session").ToLocalChecked(), ctor);
}

NAN_METHOD(SessionWrapper::New) {
  if (info.IsConstructCall()) {
    if (auto profile = SessionProfileWrapper::unwrap(info[0]).getConst()) {
      SessionWrapper *obj = new SessionWrapper();
      obj->event->session =
          genet_session_new(profile,
                            [](void *data, char *event) {
                              auto ev = static_cast<Event *>(data);
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
    Session *session = wrapper->event->session;
    uint32_t length = end - std::min(start, end);
    std::vector<const Frame *> dst;
    dst.resize(length);
    genet_session_frames(session, start, end, &length, dst.data());
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
    Session *session = wrapper->event->session;

    uint32_t length = end - std::min(start, end);
    std::vector<uint32_t> dst;
    dst.resize(length);
    genet_session_filtered_frames(session, id, start, end, &length, dst.data());

    auto array = Nan::New<v8::Array>(length);
    for (uint32_t index = 0; index < length; ++index) {
      array->Set(index, Nan::New(dst[index]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(SessionWrapper::createReader) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Second argument must be a string");
    return;
  }
  Nan::Utf8String id(info[0]);
  Nan::Utf8String arg(info[1]);

  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session *session = wrapper->event->session;
    info.GetReturnValue().Set(genet_session_create_reader(session, *id, *arg));
  }
}

NAN_METHOD(SessionWrapper::createWriter) {
  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }
  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Second argument must be a string");
    return;
  }
  if (!info[2]->IsString()) {
    Nan::ThrowTypeError("Third argument must be a string");
    return;
  }
  Nan::Utf8String id(info[0]);
  Nan::Utf8String arg(info[1]);
  Nan::Utf8String script(info[2]);

  Filter *filter = nullptr;
  if (script.length() > 0) {
    filter = FilterIsolate::createFilter(*script, script.length());
  }

  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session *session = wrapper->event->session;
    info.GetReturnValue().Set(
        genet_session_create_writer(session, *id, *arg, filter));
  }
}

NAN_METHOD(SessionWrapper::closeReader) {
  if (!info[0]->IsUint32()) {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  uint32_t handle = info[0]->Uint32Value();

  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session *session = wrapper->event->session;
    genet_session_close_reader(session, handle);
  }
}

NAN_METHOD(SessionWrapper::closeWriter) {}

NAN_METHOD(SessionWrapper::setFilter) {
  if (!info[0]->IsUint32()) {
    Nan::ThrowTypeError("First argument must be an integer");
    return;
  }
  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Second argument must be a string");
    return;
  }
  uint32_t id = info[0]->Uint32Value();
  Nan::Utf8String script(info[1]);

  Filter *filter = nullptr;
  if (script.length() > 0) {
    filter = FilterIsolate::createFilter(*script, script.length());
  }
  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session *session = wrapper->event->session;
    genet_session_set_filter(session, id, filter);
  }
}

NAN_METHOD(SessionWrapper::getMetadata) {
  void *ptr = nullptr;
  if (info[0]->IsObject()) {
    auto obj = info[0].As<v8::Object>();
    if (obj->InternalFieldCount() >= 1) {
      ptr = Nan::GetInternalFieldPointer(obj, 0);
    }
  }
  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    Session *session = wrapper->event->session;
    char *json = genet_session_get_metadata(session, ptr);
    v8::Local<v8::String> json_string = Nan::New(json).ToLocalChecked();
    genet_str_free(json);
    Nan::JSON NanJSON;
    Nan::MaybeLocal<v8::Value> result = NanJSON.Parse(json_string);
    if (!result.IsEmpty()) {
      info.GetReturnValue().Set(result.ToLocalChecked());
    }
  }
}

NAN_GETTER(SessionWrapper::callback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (!wrapper->event->callback.IsEmpty()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto func = v8::Local<v8::Function>::New(isolate, wrapper->event->callback);
    info.GetReturnValue().Set(func);
  }
}

NAN_SETTER(SessionWrapper::setCallback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (value->IsFunction()) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    wrapper->event->callback.Reset(isolate, value.As<v8::Function>());
  }
}

NAN_GETTER(SessionWrapper::length) {
  if (auto wrapper = Nan::ObjectWrap::Unwrap<SessionWrapper>(info.Holder())) {
    if (!wrapper->event) {
      Nan::ThrowReferenceError("Session has been closed");
      return;
    }
    Session *session = wrapper->event->session;
    info.GetReturnValue().Set(genet_session_len(session));
  }
}

SessionWrapper::SessionWrapper() : event(new Event()) {
  event->async.data = event;
  uv_async_init(uv_default_loop(), &event->async, [](uv_async_t *handle) {
    Event *event = reinterpret_cast<Event *>(handle->data);
    std::lock_guard<std::mutex> lock(event->mutex);

    if (!event->callback.IsEmpty()) {
      v8::Isolate *isolate = v8::Isolate::GetCurrent();
      v8::HandleScope handle_scope(isolate);
      auto global = isolate->GetCurrentContext()->Global();
      auto func = v8::Local<v8::Function>::New(isolate, event->callback);

      for (char *json : event->queue) {
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

void SessionWrapper::handleEvent(const char *event) {}

void SessionWrapper::close() {
  if (event) {
    genet_session_free(event->session);
    uv_close(reinterpret_cast<uv_handle_t *>(&event->async),
             [](uv_handle_t *handle) {
               Event *event = reinterpret_cast<Event *>(handle->data);
               delete event;
             });
    event = nullptr;
  }
}

SessionWrapper::~SessionWrapper() {}

NAN_METHOD(SessionWrapper::close) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  wrapper->close();
}

} // namespace genet_node
