#include "allocator.hpp"
#include "attr.hpp"
#include "error.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include "stream_logger.hpp"
#include "variant_map.hpp"
#include <unordered_map>

namespace plugkit {

struct Context final {
public:
  VariantMap options;
  LoggerPtr logger = std::make_shared<StreamLogger>();

  RootAllocator *rootAllocator = nullptr;
  std::unique_ptr<BlockAllocator<Frame>> frameAllocator;
  std::unique_ptr<BlockAllocator<Layer>> layerAllocator;
  std::unique_ptr<BlockAllocator<Attr>> attrAllocator;
  std::unique_ptr<BlockAllocator<Payload>> payloadAllocator;
  std::unique_ptr<BlockAllocator<Error>> errorAllocator;

  void *data = nullptr;
};

Frame *Context_allocFrame(Context *ctx);
void Context_deallocFrame(Context *ctx, Frame *frame);
Layer *Context_allocLayer(Context *ctx, Token id);
void Context_deallocLayer(Context *ctx, Layer *layer);
Attr *Context_allocAttr(Context *ctx, Token id);
void Context_deallocAttr(Context *ctx, Attr *attr);
Payload *Context_allocPayload(Context *ctx);
void Context_deallocPayload(Context *ctx, Payload *payload);
Error *Context_allocError(Context *ctx);
void Context_deallocError(Context *ctx, Error *error);

extern "C" {
/// Allocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `malloc()`.
void *Context_alloc(Context *ctx, size_t size);

/// Reallocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `realloc()`.
void *Context_realloc(Context *ctx, void *ptr, size_t size);

/// Deallocate a memory block in the current context.
/// @remark Currently, this function is just a wrapper for `free()`.
void Context_dealloc(Context *ctx, void *ptr);

/// Return the value of the option in the current context.
const Variant *Context_getOption(Context *ctx, const char *key);
}

} // namespace plugkit
