#include "../src/session.hpp"
#include "plugkit_module.hpp"
#include "session.hpp"

#include "wrapper/frame.hpp"

namespace plugkit {

void SessionWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Session").ToLocalChecked());
  SetPrototypeMethod(tpl, "startPcap", startPcap);
  SetPrototypeMethod(tpl, "stopPcap", stopPcap);
  SetPrototypeMethod(tpl, "destroy", destroy);
  SetPrototypeMethod(tpl, "getFilteredFrames", getFilteredFrames);
  SetPrototypeMethod(tpl, "getFrames", getFrames);
  SetPrototypeMethod(tpl, "analyze", analyze);
  SetPrototypeMethod(tpl, "setDisplayFilter", setDisplayFilter);
  SetPrototypeMethod(tpl, "setStatusCallback", setStatusCallback);
  SetPrototypeMethod(tpl, "setFilterCallback", setFilterCallback);
  SetPrototypeMethod(tpl, "setFrameCallback", setFrameCallback);
  SetPrototypeMethod(tpl, "setLoggerCallback", setLoggerCallback);

  PlugkitModule *module = PlugkitModule::get(isolate);
  module->session.ctor.Reset(isolate, Nan::GetFunction(tpl).ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("networkInterface").ToLocalChecked(),
                   networkInterface);
  Nan::SetAccessor(otl, Nan::New("promiscuous").ToLocalChecked(), promiscuous);
  Nan::SetAccessor(otl, Nan::New("snaplen").ToLocalChecked(), snaplen);
  Nan::SetAccessor(otl, Nan::New("options").ToLocalChecked(), options);
  Nan::SetAccessor(otl, Nan::New("id").ToLocalChecked(), id);
}

SessionWrapper::SessionWrapper(const std::shared_ptr<Session> &session)
    : session(session) {}

SessionWrapper::~SessionWrapper() {}

NAN_METHOD(SessionWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_GETTER(SessionWrapper::networkInterface) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(
        Nan::New(session->networkInterface()).ToLocalChecked());
  }
}

NAN_GETTER(SessionWrapper::promiscuous) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(session->promiscuous());
  }
}

NAN_GETTER(SessionWrapper::snaplen) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(session->snaplen());
  }
}

NAN_METHOD(SessionWrapper::startPcap) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(session->startPcap());
  }
}

NAN_METHOD(SessionWrapper::stopPcap) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(session->stopPcap());
  }
}

NAN_GETTER(SessionWrapper::id) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(session->id());
  }
}

NAN_GETTER(SessionWrapper::options) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    info.GetReturnValue().Set(Variant::getValue(session->options()));
  }
}

NAN_METHOD(SessionWrapper::destroy) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  wrapper->session.reset();
  wrapper->statusCallback.Reset();
  wrapper->filterCallback.Reset();
  wrapper->frameCallback.Reset();
  wrapper->loggerCallback.Reset();
}

NAN_METHOD(SessionWrapper::getFilteredFrames) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    const std::string &name = *Nan::Utf8String(info[0]);
    uint32_t offset = info[1]->Uint32Value();
    uint32_t length = info[2]->Uint32Value();
    const auto &frames = session->getFilteredFrames(name, offset, length);
    auto array = Nan::New<v8::Array>(frames.size());
    for (size_t i = 0; i < frames.size(); ++i) {
      array->Set(i, Nan::New(frames[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(SessionWrapper::getFrames) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    uint32_t offset = info[0]->Uint32Value();
    uint32_t length = info[1]->Uint32Value();
    const auto &frames = session->getFrames(offset, length);
    auto array = Nan::New<v8::Array>(frames.size());
    for (size_t i = 0; i < frames.size(); ++i) {
      array->Set(i, FrameWrapper::wrap(frames[i]));
    }
    info.GetReturnValue().Set(array);
  }
}

NAN_METHOD(SessionWrapper::analyze) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    if (info[0]->IsArray()) {
      auto array = info[0].As<v8::Array>();
      std::vector<Session::RawFrame> frames;
      for (uint32_t i = 0; i < array->Length(); ++i) {
        auto obj = array->Get(i).As<v8::Object>();
        Session::RawFrame frame;
        frame.link = obj->Get(Nan::New("link").ToLocalChecked())->Uint32Value();

        auto payload = obj->Get(Nan::New("payload").ToLocalChecked());
        if (node::Buffer::HasInstance(payload)) {
          size_t len = node::Buffer::Length(payload);
          char *data = new char[len];
          std::memcpy(data, node::Buffer::Data(payload), len);
          frame.payload = Slice{data, data + len};
        }
        frame.length =
            obj->Get(Nan::New("length").ToLocalChecked())->Uint32Value();

        auto timestamp = obj->Get(Nan::New("timestamp").ToLocalChecked());
        if (timestamp->IsDate()) {
          auto nsec = timestamp.As<v8::Object>()->Get(
              Nan::New("nsec").ToLocalChecked());
          uint64_t ts =
              static_cast<uint64_t>(timestamp.As<v8::Date>()->ValueOf()) *
              1000000;
          if (nsec->IsNumber()) {
            ts += nsec->NumberValue();
          }
          frame.timestamp = Timestamp(std::chrono::nanoseconds(ts));
        } else {
          frame.timestamp = std::chrono::system_clock::now();
        }

        frame.sourceId =
            obj->Get(Nan::New("sourceId").ToLocalChecked())->Uint32Value();
        frames.push_back(std::move(frame));
      }
      session->analyze(frames);
    }
  }
}

NAN_METHOD(SessionWrapper::setDisplayFilter) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    const std::string &name = *Nan::Utf8String(info[0]);
    const std::string &body = *Nan::Utf8String(info[1]);
    session->setDisplayFilter(name, body);
  }
}

NAN_METHOD(SessionWrapper::setStatusCallback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    if (info[0]->IsFunction()) {
      wrapper->statusCallback.Reset(v8::Isolate::GetCurrent(),
                                    info[0].As<v8::Function>());

      session->setStatusCallback([wrapper](const Session::Status &status) {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        auto func =
            v8::Local<v8::Function>::New(isolate, wrapper->statusCallback);
        if (!func.IsEmpty()) {
          auto obj = Nan::New<v8::Object>();
          obj->Set(Nan::New("capture").ToLocalChecked(),
                   Nan::New(status.capture));
          v8::Local<v8::Value> args[1] = {obj};
          func->Call(obj, 1, args);
        }
      });
    }
  }
}

NAN_METHOD(SessionWrapper::setFilterCallback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    if (info[0]->IsFunction()) {
      wrapper->filterCallback.Reset(v8::Isolate::GetCurrent(),
                                    info[0].As<v8::Function>());
      session->setFilterCallback(
          [wrapper](const Session::FilterStatusMap &status) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            auto func =
                v8::Local<v8::Function>::New(isolate, wrapper->filterCallback);
            if (!func.IsEmpty()) {
              auto obj = Nan::New<v8::Object>();

              for (const auto &pair : status) {
                auto filter = Nan::New<v8::Object>();
                filter->Set(Nan::New("frames").ToLocalChecked(),
                            Nan::New(pair.second.frames));
                obj->Set(Nan::New(pair.first).ToLocalChecked(), filter);
              }

              v8::Local<v8::Value> args[1] = {obj};
              func->Call(obj, 1, args);
            }
          });
    }
  }
}

NAN_METHOD(SessionWrapper::setFrameCallback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    if (info[0]->IsFunction()) {
      wrapper->frameCallback.Reset(v8::Isolate::GetCurrent(),
                                   info[0].As<v8::Function>());
      session->setFrameCallback([wrapper](const Session::FrameStatus &status) {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        auto func =
            v8::Local<v8::Function>::New(isolate, wrapper->frameCallback);
        if (!func.IsEmpty()) {
          auto obj = Nan::New<v8::Object>();
          obj->Set(Nan::New("frames").ToLocalChecked(),
                   Nan::New(status.frames));
          v8::Local<v8::Value> args[1] = {obj};
          func->Call(obj, 1, args);
        }
      });
    }
  }
}

NAN_METHOD(SessionWrapper::setLoggerCallback) {
  SessionWrapper *wrapper = ObjectWrap::Unwrap<SessionWrapper>(info.Holder());
  if (const auto &session = wrapper->session) {
    if (info[0]->IsFunction()) {
      wrapper->loggerCallback.Reset(v8::Isolate::GetCurrent(),
                                    info[0].As<v8::Function>());
      session->setLoggerCallback([wrapper](Logger::MessagePtr &&msg) {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        auto func =
            v8::Local<v8::Function>::New(isolate, wrapper->loggerCallback);
        if (!func.IsEmpty()) {
          auto obj = Nan::New<v8::Object>();
          obj->Set(Nan::New("message").ToLocalChecked(),
                   Nan::New(msg->toString()).ToLocalChecked());
          v8::Local<v8::Value> args[1] = {obj};
          func->Call(obj, 1, args);
        }
      });
    }
  }
}

v8::Local<v8::Object>
SessionWrapper::wrap(const std::shared_ptr<Session> &session) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->session.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  SessionWrapper *wrapper = new SessionWrapper(session);
  wrapper->Wrap(obj);
  return obj;
}

std::shared_ptr<const Session>
SessionWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  if (auto wrapper =
          ObjectWrap::Unwrap<SessionWrapper>(value.As<v8::Object>())) {
    return wrapper->session;
  }
  return std::shared_ptr<Session>();
}
} // namespace plugkit
