#include "context.h"
#include "variant.hpp"
#include "stream_logger.hpp"

namespace plugkit {

struct Context final {
public:
  Variant options;
  std::vector<std::pair<Layer *, std::string>> streamedLayers;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
}
