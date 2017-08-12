#include <cstdio>
#include <iostream>
#include "context.hpp"
#include "logger.h"
#include "logger.hpp"

namespace plugkit {

void *Context_alloc(Context *ctx, size_t size) {
  // zero initialized
  return new char[size]();
}

void Context_dealloc(Context *ctx, void *ptr) {
  delete[] static_cast<char *>(ptr);
}

const Variant *Context_options(Context *ctx) { return &ctx->options; }

void Log_debug_(Context *ctx, const char *file, int line, const char *format,
                ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Logger::Message msg;
  msg.level = Logger::LEVEL_DEBUG;
  msg.message = buffer;
  msg.resourceName = file;
  msg.lineNumber = line;
  msg.trivial = true;

  std::cerr << msg.toString() << std::endl;
}
void Log_warn_(Context *ctx, const char *file, int line, const char *format,
               ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Logger::Message msg;
  msg.level = Logger::LEVEL_WARN;
  msg.message = buffer;
  msg.resourceName = file;
  msg.lineNumber = line;
  msg.trivial = true;

  std::cerr << msg.toString() << std::endl;
}
void Log_info_(Context *ctx, const char *file, int line, const char *format,
               ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Logger::Message msg;
  msg.level = Logger::LEVEL_INFO;
  msg.message = buffer;
  msg.resourceName = file;
  msg.lineNumber = line;
  msg.trivial = true;

  std::cerr << msg.toString() << std::endl;
}
void Log_error_(Context *ctx, const char *file, int line, const char *format,
                ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Logger::Message msg;
  msg.level = Logger::LEVEL_ERROR;
  msg.message = buffer;
  msg.resourceName = file;
  msg.lineNumber = line;
  msg.trivial = true;

  std::cerr << msg.toString() << std::endl;
}
}
