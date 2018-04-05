#include "allocator.hpp"
#include "attr.hpp"
#include "config_map.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include "stream_logger.hpp"
#include "string_pool.hpp"
#include <unordered_map>

namespace std {
template <>
struct hash<std::pair<plugkit::Token, uint64_t>> {
  inline size_t operator()(const pair<plugkit::Token, uint64_t> &v) const {
    return v.first + v.second;
  }
};
} // namespace std

namespace plugkit {

class SessionContext;
struct SharedContextWrapper;

struct Context final {
public:
  Context(const SessionContext *sctx);

  bool closeStream = false;
  uint32_t confidenceThreshold = 0;

  SharedContextWrapper *shared;
  const SessionContext *sctx;

  struct PrevLayer {
    Layer *layer;
    Attr *attr;
  };
  std::unordered_map<std::pair<Token, uint64_t>, PrevLayer> linkedLayers;

  std::unique_ptr<BlockAllocator<Frame>> frameAllocator;
  std::unique_ptr<BlockAllocator<Layer>> layerAllocator;
  std::unique_ptr<BlockAllocator<Attr>> attrAllocator;
  std::unique_ptr<BlockAllocator<Payload>> payloadAllocator;

  StringPool stringPool;

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

extern "C" {
/// Return the value of the option in the current context.
const char *Context_getConfig(Context *ctx, const char *key, size_t length);

void Context_closeStream(Context *ctx);

void Context_addLayerLinkage(Context *ctx,
                             Token token,
                             uint64_t id,
                             Layer *layer);
}

} // namespace plugkit
