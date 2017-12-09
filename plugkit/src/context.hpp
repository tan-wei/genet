#include "allocator.hpp"
#include "attr.hpp"
#include "context.h"
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
  VariantMap variables;
  LoggerPtr logger = std::make_shared<StreamLogger>();

  RootAllocator *rootAllocator = nullptr;
  std::unique_ptr<BlockAllocator<Frame>> frameAllocator;
  std::unique_ptr<BlockAllocator<Layer>> layerAllocator;
  std::unique_ptr<BlockAllocator<Attr>> attrAllocator;
  std::unique_ptr<BlockAllocator<Payload>> payloadAllocator;

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

} // namespace plugkit
