#include "context.hpp"
#include "attr.hpp"
#include <cstdarg>
#include <cstdio>

namespace plugkit {

namespace {
const auto nextToken = Token_get("--next");
const auto prevToken = Token_get("--prev");
} // namespace

void *Context_alloc(Context *ctx, size_t size) { return malloc(size); }

void *Context_realloc(Context *ctx, void *ptr, size_t size) {
  return realloc(ptr, size);
}

void Context_dealloc(Context *ctx, void *ptr) { free(ptr); }

Frame *Context_allocFrame(Context *ctx) {
  if (!ctx->rootAllocator)
    return nullptr;
  if (!ctx->frameAllocator) {
    ctx->frameAllocator.reset(new BlockAllocator<Frame>(ctx->rootAllocator));
  }
  return ctx->frameAllocator->alloc();
}

void Context_deallocFrame(Context *ctx, Frame *frame) {
  if (!ctx->rootAllocator)
    return;
  ctx->frameAllocator->dealloc(frame);
}

Layer *Context_allocLayer(Context *ctx, Token id) {
  if (!ctx->rootAllocator)
    return nullptr;
  if (!ctx->layerAllocator) {
    ctx->layerAllocator.reset(new BlockAllocator<Layer>(ctx->rootAllocator));
  }
  return ctx->layerAllocator->alloc(id);
}

void Context_deallocLayer(Context *ctx, Layer *layer) {
  if (!ctx->rootAllocator)
    return;
  ctx->layerAllocator->dealloc(layer);
}

Attr *Context_allocAttr(Context *ctx, Token id) {
  if (!ctx->rootAllocator)
    return nullptr;
  if (!ctx->attrAllocator) {
    ctx->attrAllocator.reset(new BlockAllocator<Attr>(ctx->rootAllocator));
  }
  return ctx->attrAllocator->alloc(id);
}

void Context_deallocAttr(Context *ctx, Attr *attr) {
  if (!ctx->rootAllocator)
    return;
  ctx->attrAllocator->dealloc(attr);
}

Payload *Context_allocPayload(Context *ctx) {
  if (!ctx->rootAllocator)
    return nullptr;
  if (!ctx->payloadAllocator) {
    ctx->payloadAllocator.reset(
        new BlockAllocator<Payload>(ctx->rootAllocator));
  }
  return ctx->payloadAllocator->alloc();
}

void Context_deallocPayload(Context *ctx, Payload *payload) {
  if (!ctx->rootAllocator)
    return;
  ctx->payloadAllocator->dealloc(payload);
}

const Variant *Context_getOption(Context *ctx, const char *key, size_t length) {
  const Variant &value = ctx->options[std::string(key, length)];
  if (!value.isNil()) {
    return &value;
  }
  static const Variant null;
  return &null;
}

void Context_closeStream(Context *ctx) { ctx->closeStream = true; }

void Context_addLayerLinkage(Context *ctx,
                             Token token,
                             uint64_t id,
                             Layer *layer) {
  const auto &pair = std::make_pair(token, id);
  auto it = ctx->linkedLayers.find(pair);
  if (it != ctx->linkedLayers.end()) {
    {
      Attr *attr = Context_allocAttr(ctx, nextToken);
      attr->setType(nextToken);
      attr->setValue(Variant::fromAddress(layer));
      it->second->addAttr(attr);
    }
    {
      Attr *attr = Context_allocAttr(ctx, prevToken);
      attr->setType(prevToken);
      attr->setValue(Variant::fromAddress(it->second));
      layer->addAttr(attr);
    }
  }
  ctx->linkedLayers[pair] = layer;
}

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
