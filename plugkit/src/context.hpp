#include "allocator.hpp"
#include "context.h"
#include "layer.hpp"
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
  std::unique_ptr<BlockAllocator<Layer>> layerAllocator;

  void *data = nullptr;
};
} // namespace plugkit
