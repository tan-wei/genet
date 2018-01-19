#include "../src/session.hpp"
#include "dissector.hpp"
#include "file.hpp"
#include "plugkit_module.hpp"
#include "session.hpp"
#include "session_factory.hpp"

#include "module_loader.hpp"

namespace plugkit {

void SessionFactoryWrapper::init(v8::Isolate *isolate,
                                 v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("SessionFactory").ToLocalChecked());
  SetPrototypeMethod(tpl, "setOption", setOption);
  SetPrototypeMethod(tpl, "registerDissector", registerDissector);
  SetPrototypeMethod(tpl, "registerLinkLayer", registerLinkLayer);
  SetPrototypeMethod(tpl, "registerImporter", registerImporter);
  SetPrototypeMethod(tpl, "registerExporter", registerExporter);
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

NAN_METHOD(SessionFactoryWrapper::setOption) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (auto factory = wrapper->factory) {
    std::string key = *Nan::Utf8String(info[0]);
    const Variant &value = Variant::getVariant(info[1]);
    factory->setOption(key, value);
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
    DissectorType type = DISSECTOR_PACKET;
    if (std::strcmp("stream", *Nan::Utf8String(info[1])) == 0) {
      type = DISSECTOR_STREAM;
    }
    if (info[0]->IsString()) {
      const std::string &path = *Nan::Utf8String(info[0]);
      if (path.rfind(".node") == path.size() - 5) {
        Dissector diss = {0};
        ModuleLoader loader(path);
        if (auto func = loader.load<AnalyzeFunc*>("plugkit_v1_analyze")) {
          diss.analyze = func;
        }
        if (auto func = loader.load<CreateWorkerFunc*>("plugkit_v1_create_worker")) {
          diss.createWorker = func;
        }
        if (auto func = loader.load<DestroyWorkerFunc*>("plugkit_v1_destroy_worker")) {
          diss.destroyWorker = func;
        }
        if (auto func = loader.load<Token(*)(int)>("plugkit_v1_layer_hints")) {
          for (size_t i = 0; i < sizeof(diss.layerHints) / sizeof(Token); ++i) {
            if (!(diss.layerHints[i] = func(i))) break;
          }
        }
        factory->registerDissector(diss, type);
      } else {
        factory->registerDissector(path, type);
      }
    }
  }
}

NAN_METHOD(SessionFactoryWrapper::registerImporter) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    if (info[0]->IsString()) {
      const std::string &path = *Nan::Utf8String(info[0]);
      if (path.rfind(".node") == path.size() - 5) {
        FileImporter importer = {0};
        ModuleLoader loader(path);
        if (auto func = loader.load<FileImporterFunc*>("plugkit_v1_file_import")) {
          factory->registerImporter(FileImporter{func});
        }
      }
    }
  }
}

NAN_METHOD(SessionFactoryWrapper::registerExporter) {
  SessionFactoryWrapper *wrapper =
      ObjectWrap::Unwrap<SessionFactoryWrapper>(info.Holder());
  if (const auto &factory = wrapper->factory) {
    if (info[0]->IsString()) {
      const std::string &path = *Nan::Utf8String(info[0]);
      if (path.rfind(".node") == path.size() - 5) {
        FileExporter exporter = {0};
        ModuleLoader loader(path);
        if (auto func = loader.load<FileExporterFunc*>("plugkit_v1_file_export")) {
          factory->registerExporter(FileExporter{func});
        }
      }
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

SessionFactoryPtr SessionFactoryWrapper::unwrap(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsObject())
    return nullptr;
  if (auto wrapper =
          ObjectWrap::Unwrap<SessionFactoryWrapper>(value.As<v8::Object>())) {
    return wrapper->factory;
  }
  return SessionFactoryPtr();
}
} // namespace plugkit
