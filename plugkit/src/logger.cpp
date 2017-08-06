#include "logger.hpp"
#include <iomanip>
#include <mutex>
#include <nan.h>
#include <sstream>
#include <uv.h>
#include <vector>

namespace plugkit {

std::string Logger::Message::levelString() const {
  switch (level) {
  case LEVEL_DEBUG:
    return "debug";
  case LEVEL_INFO:
    return "info";
  case LEVEL_WARN:
    return "warn";
  case LEVEL_ERROR:
    return "error";
  }
  return "unknown";
}

std::string Logger::Message::toString() const {
  std::stringstream stream;
  std::time_t ts = std::chrono::system_clock::to_time_t(timestamp);
  std::tm tm = *std::localtime(&ts);
  stream << "[" << levelString() << "] " << std::put_time(&tm, "%T") << " #"
         << threadId << " (" << domain << ") " << message;
  return stream.str();
}

Logger::~Logger() {}

void Logger::log(Level level, const std::string &message,
                 const std::string &domain, const std::string &resourceName) {
  auto msg = MessagePtr(new Message());
  msg->level = level;
  msg->message = message;
  msg->domain = domain;
  msg->resourceName = resourceName;
  log(std::move(msg));
}

void Logger::logTrivial(Level level, const std::string &message,
                        const std::string &domain,
                        const std::string &resourceName) {

  auto msg = MessagePtr(new Message());
  msg->level = level;
  msg->message = message;
  msg->domain = domain;
  msg->resourceName = resourceName;
  msg->trivial = true;
  log(std::move(msg));
}

Logger::MessagePtr Logger::fromV8Message(v8::Local<v8::Message> msg,
                                         Logger::Level level,
                                         const std::string &domain) {

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  auto context = isolate->GetCurrentContext();
  MessagePtr logmsg(new Logger::Message());
  logmsg->level = level;
  logmsg->domain = domain;
  logmsg->message = *Nan::Utf8String(msg->Get());
  logmsg->resourceName = *Nan::Utf8String(msg->GetScriptResourceName());
  logmsg->sourceLine = *Nan::Utf8String(msg->GetSourceLine());
  logmsg->lineNumber = msg->GetLineNumber();
  logmsg->startPosition = msg->GetStartPosition();
  logmsg->endPosition = msg->GetEndPosition();
  logmsg->startColumn = msg->GetStartColumn(context).FromMaybe(0);
  logmsg->endColumn = msg->GetEndColumn(context).FromMaybe(0);
  return logmsg;
}
}
