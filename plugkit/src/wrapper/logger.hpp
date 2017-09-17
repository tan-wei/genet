#ifndef PLUGKIT_LOGGER_WRAPPER_H
#define PLUGKIT_LOGGER_WRAPPER_H

#include <memory>
#include <nan.h>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class LoggerWrapper final : public Nan::ObjectWrap {
public:
  static void init(v8::Isolate *isolate);
  static NAN_METHOD(New);
  static NAN_METHOD(log);
  static NAN_METHOD(debug);
  static NAN_METHOD(warn);
  static NAN_METHOD(error);
  static v8::Local<v8::Object> wrap(const LoggerPtr &logger);

private:
  LoggerWrapper(const LoggerPtr &logger);
  LoggerWrapper(const LoggerWrapper &) = delete;
  LoggerWrapper &operator=(const LoggerWrapper &) = delete;

private:
  LoggerPtr logger;
};
}

#endif
