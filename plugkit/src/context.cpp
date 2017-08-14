#include <cstdio>
#include "context.hpp"
#include "logger.h"

namespace plugkit {

const Variant *Context_options(Context *ctx) { return &ctx->options; }

void Context_addStreamIdentifier(Context *ctx, Layer *layer, const char *data,
                                 size_t length) {
  ctx->streamedLayers.push_back(
      std::make_pair(layer, std::string(data, length)));
}

namespace {
void Log(Context *ctx, const char *file, int line, Logger::Level level,
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
}

void Log_debug_(Context *ctx, const char *file, int line, const char *format,
                ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_DEBUG, buffer);
}
void Log_warn_(Context *ctx, const char *file, int line, const char *format,
               ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_WARN, buffer);
}
void Log_info_(Context *ctx, const char *file, int line, const char *format,
               ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_INFO, buffer);
}
void Log_error_(Context *ctx, const char *file, int line, const char *format,
                ...) {
  char buffer[2048];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Log(ctx, file, line, Logger::LEVEL_ERROR, buffer);
}
}
