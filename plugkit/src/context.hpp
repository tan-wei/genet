#include "context.h"
#include "stream_logger.hpp"
#include "variant.hpp"

namespace plugkit {

struct Context final {
public:
  Variant options;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
} // namespace plugkit
