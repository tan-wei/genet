#include "context.hpp"
#include "logger.h"
#include <cstdio>

namespace plugkit {

void *Context_alloc(Context *ctx, size_t size) { return malloc(size); }

void *Context_realloc(Context *ctx, void *ptr, size_t size) {
  return realloc(ptr, size);
}

void Context_dealloc(Context *ctx, void *ptr) { free(ptr); }

const Variant *Context_options(Context *ctx) { return &ctx->options; }

namespace {
void Log(Context *ctx,
         const char *file,
         int line,
         Logger::Level level,
         const char *message) {
  Logger::MessagePtr msg(new Logger::Message());
  msg->level = level;
  msg->domain = "core:dissector";
  msg->message = message;
  msg->resourceName = file;
  msg->lineNumber = line;
  msg->trivial = level != Logger::LEVEL_ERROR;
  if (ctx->logger) {
    ctx->logger->log(std::move(msg));
  }
}
} // namespace

void Log_debug_(
    Context *ctx, const char *file, int line, const char *format, ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_DEBUG, buffer);
}
void Log_warn_(
    Context *ctx, const char *file, int line, const char *format, ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_WARN, buffer);
}
void Log_info_(
    Context *ctx, const char *file, int line, const char *format, ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_INFO, buffer);
}
void Log_error_(
    Context *ctx, const char *file, int line, const char *format, ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_ERROR, buffer);
}
} // namespace plugkit
