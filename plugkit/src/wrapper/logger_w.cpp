#include "wrapper/logger.hpp"
#include "../logger.hpp"
#include "plugkit_module.hpp"

namespace plugkit {

void LoggerWrapper::init(v8::Isolate *isolate) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Logger").ToLocalChecked());
  SetPrototypeMethod(tpl, "log", log);
  SetPrototypeMethod(tpl, "info", log);
  SetPrototypeMethod(tpl, "debug", debug);
  SetPrototypeMethod(tpl, "warn", warn);
  SetPrototypeMethod(tpl, "error", error);

  PlugkitModule *module = PlugkitModule::get(isolate);
  auto ctor = Nan::GetFunction(tpl).ToLocalChecked();
  module->logger.proto.Reset(isolate,
                             ctor->Get(Nan::New("prototype").ToLocalChecked()));
  module->logger.ctor.Reset(isolate, ctor);
}

NAN_METHOD(LoggerWrapper::New) { info.GetReturnValue().Set(info.This()); }

NAN_METHOD(LoggerWrapper::log) {
  LoggerWrapper *wrapper = ObjectWrap::Unwrap<LoggerWrapper>(info.Holder());
  if (const auto &logger = wrapper->logger) {
    logger->logTrivial(Logger::LEVEL_INFO, *Nan::Utf8String(info[0]), "worker");
  }
}
NAN_METHOD(LoggerWrapper::debug) {
  LoggerWrapper *wrapper = ObjectWrap::Unwrap<LoggerWrapper>(info.Holder());
  if (const auto &logger = wrapper->logger) {
    logger->logTrivial(Logger::LEVEL_DEBUG, *Nan::Utf8String(info[0]),
                       "worker");
  }
}
NAN_METHOD(LoggerWrapper::warn) {
  LoggerWrapper *wrapper = ObjectWrap::Unwrap<LoggerWrapper>(info.Holder());
  if (const auto &logger = wrapper->logger) {
    logger->logTrivial(Logger::LEVEL_WARN, *Nan::Utf8String(info[0]), "worker");
  }
}
NAN_METHOD(LoggerWrapper::error) {
  LoggerWrapper *wrapper = ObjectWrap::Unwrap<LoggerWrapper>(info.Holder());
  if (const auto &logger = wrapper->logger) {
    logger->logTrivial(Logger::LEVEL_ERROR, *Nan::Utf8String(info[0]),
                       "worker");
  }
}

LoggerWrapper::LoggerWrapper(const LoggerPtr &logger) : logger(logger) {}

v8::Local<v8::Object> LoggerWrapper::wrap(const LoggerPtr &logger) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  PlugkitModule *module = PlugkitModule::get(isolate);
  auto cons = v8::Local<v8::Function>::New(isolate, module->logger.ctor);
  v8::Local<v8::Object> obj =
      cons->NewInstance(v8::Isolate::GetCurrent()->GetCurrentContext(), 0,
                        nullptr)
          .ToLocalChecked();
  LoggerWrapper *wrapper = new LoggerWrapper(logger);
  wrapper->Wrap(obj);
  return obj;
}
}
