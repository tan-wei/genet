#include "session_factory.hpp"
#include "script_dissector.hpp"
#include "script_stream_dissector.hpp"
#include "dissector_factory.hpp"
#include "stream_dissector_factory.hpp"
#include "session.hpp"
#include "private/variant.hpp"
#include "../src/session.hpp"
#include "plugkit_module.hpp"
#include "extended_slot.hpp"

namespace plugkit {

void SessionFactoryWrapper::init(v8::Isolate *isolate,
                                 v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("SessionFactory").ToLocalChecked());
  SetPrototypeMethod(tpl, "registerDissector", registerDissector);
  SetPrototypeMethod(tpl, "registerStreamDissector", registerStreamDissector);
  SetPrototypeMethod(tpl, "registerLinkLayer", registerLinkLayer);
  SetPrototypeMethod(tpl, "create", create);

  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  module->sessionFactory.ctor.Reset(isolate,
                                    Nan::GetFunction(tpl).ToLocalChecked());

  v8::Local<v8::ObjectTemplate> otl = tpl->InstanceTemplate();
  Nan::SetAccessor(otl, Nan::New("networkInterface").ToLocalChecked(),
                   networkInterface, setNetworkInterface);
  Nan::SetAccessor(otl, Nan::New("promiscuous").ToLocalChecked(), promiscuous,
                   setPromiscuous);
  Nan::SetAccessor(otl, Nan::New("snaplen").ToLocalChecked(), snaplen,
                   setSnaplen);
  Nan::SetAccessor(otl, Nan::New("bpf").ToLocalChecked(), bpf, setBpf);
  Nan::SetAccessor(otl, Nan::New("options").ToLocalChecked(), options,
                   setOptions);

  v8::Local<v8::Object> func = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, Nan::New("SessionFactory").ToLocalChecked(), func);
}

SessionFactoryWrapper::SessionFactoryWrapper(const SessionFactoryPtr &factory)
    : factory(factory) {}

NAN_METHOD(SessionFactoryWrapper::New) {
  SessionFactoryWrapper *obj =
      new SessionFactoryWrapper(std::make_shared<SessionFactory>());
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(SessionFactoryWrapper::networkInterface) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    info.GetReturnValue().Set(
        Nan::New(factory->networkInterface()).ToLocalChecked());
  }
}

NAN_SETTER(SessionFactoryWrapper::setNetworkInterface) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    factory->setNetworkInterface(*Nan::Utf8String(value));
  }
}

NAN_GETTER(SessionFactoryWrapper::promiscuous) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    info.GetReturnValue().Set(factory->promiscuous());
  }
}

NAN_SETTER(SessionFactoryWrapper::setPromiscuous) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    factory->setPromiscuous(value->BooleanValue());
  }
}

NAN_GETTER(SessionFactoryWrapper::snaplen) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    info.GetReturnValue().Set(factory->snaplen());
  }
}

NAN_SETTER(SessionFactoryWrapper::setSnaplen) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    factory->setSnaplen(value->IntegerValue());
  }
}

NAN_GETTER(SessionFactoryWrapper::bpf) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    info.GetReturnValue().Set(Nan::New(factory->bpf()).ToLocalChecked());
  }
}

NAN_SETTER(SessionFactoryWrapper::setBpf) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    const std::string &bpf = *Nan::Utf8String(value);
    factory->setBpf(bpf);
  }
}

NAN_GETTER(SessionFactoryWrapper::options) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (auto factory = wrapper->factory) {
    info.GetReturnValue().Set(Variant::Private::getValue(factory->options()));
  }
}

NAN_SETTER(SessionFactoryWrapper::setOptions) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (auto factory = wrapper->factory) {
    factory->setOptions(Variant::Private::getVariant(value));
  }
}

NAN_METHOD(SessionFactoryWrapper::registerLinkLayer) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    int link = info[0]->IntegerValue();
    const std::string &ns = *Nan::Utf8String(info[1]);
    const std::string &name = *Nan::Utf8String(info[2]);
    factory->registerLinkLayer(link, ns, name);
  }
}

NAN_METHOD(SessionFactoryWrapper::registerDissector) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    DissectorFactoryConstPtr dissectorFactory;

    if (info[0]->IsString()) {
      const std::string &script = *Nan::Utf8String(info[0]);
      const std::string &path = *Nan::Utf8String(info[1]);
      dissectorFactory = std::make_shared<ScriptDissectorFactory>(script, path);
    } else if (info[0]->IsObject()) {
      dissectorFactory =
          DissectorFactoryWrapper::unwrap(info[0].As<v8::Object>());
    }
    if (dissectorFactory) {
      factory->registerDissector(dissectorFactory);
    }
  }
}

NAN_METHOD(SessionFactoryWrapper::registerStreamDissector) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    StreamDissectorFactoryConstPtr dissectorFactory;

    if (info[0]->IsString()) {
      const std::string &script = *Nan::Utf8String(info[0]);
      const std::string &path = *Nan::Utf8String(info[1]);
      dissectorFactory =
          std::make_shared<ScriptStreamDissectorFactory>(script, path);
    } else if (info[0]->IsObject()) {
      dissectorFactory =
          StreamDissectorFactoryWrapper::unwrap(info[0].As<v8::Object>());
    }
    if (dissectorFactory) {
      factory->registerStreamDissector(dissectorFactory);
    }
  }
}

NAN_METHOD(SessionFactoryWrapper::create) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    info.GetReturnValue().Set(SessionWrapper::wrap(factory->create()));
  }
}

v8::Local<v8::Object>
SessionFactoryWrapper::wrap(const SessionFactoryPtr &factory) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = ExtendedSlot::get<PlugkitModule>(
      isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE);
  auto cons =
      v8::Local<v8::Function>::New(isolate, module->sessionFactory.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  SessionFactoryWrapper *wrapper = new SessionFactoryWrapper(factory);
  wrapper->Wrap(obj);
  return obj;
}

SessionFactoryPtr SessionFactoryWrapper::unwrap(v8::Local<v8::Object> obj) {
  if (auto wrapper = ObjectWrap::Unwrap<SessionFactoryWrapper>(obj)) {
    return wrapper->factory;
  }
  return SessionFactoryPtr();
}
}
