#include "../src/session.hpp"
#include "plugkit_module.hpp"
#include "session_factory.hpp"

#include "session.hpp"

namespace plugkit {

void SessionFactoryWrapper::init(v8::Isolate *isolate,
                                 v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("SessionFactory").ToLocalChecked());
  SetPrototypeMethod(tpl, "registerDissector", registerDissector);
  SetPrototypeMethod(tpl, "registerLinkLayer", registerLinkLayer);
  SetPrototypeMethod(tpl, "create", create);

  PlugkitModule *module = PlugkitModule::get(isolate);
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
    info.GetReturnValue().Set(Variant::getValue(factory->options()));
  }
}

NAN_SETTER(SessionFactoryWrapper::setOptions) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (auto factory = wrapper->factory) {
    factory->setOptions(Variant::getVariant(value));
  }
}

NAN_METHOD(SessionFactoryWrapper::registerLinkLayer) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    int link = info[0]->IntegerValue();
    factory->registerLinkLayer(link, Token_get(*Nan::Utf8String(info[1])));
  }
}

NAN_METHOD(SessionFactoryWrapper::registerDissector) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    const Dissector *dissector = nullptr;

    if (info[0]->IsExternal()) {
      dissector =
          static_cast<const Dissector *>(info[0].As<v8::External>()->Value());
    }
    if (dissector) {
      factory->registerDissector(*dissector);
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
  PlugkitModule *module = PlugkitModule::get(isolate);
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
} // namespace plugkit
