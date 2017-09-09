#include "context.h"
#include "variant.hpp"
#include "stream_logger.hpp"

namespace plugkit {

struct Context final {
public:
  Variant options;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
}
