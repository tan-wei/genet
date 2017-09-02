#include "context.h"
#include "variant.hpp"
#include "stream_logger.hpp"

namespace plugkit {

struct Context final {
public:
  Variant options;
  std::map<Layer *, Variant> streamedLayers;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
}
