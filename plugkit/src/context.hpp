#include "context.h"
#include "stream_logger.hpp"
#include "variant.hpp"
#include <unordered_map>

namespace plugkit {

struct Context final {
public:
  Variant options;
  std::unordered_map<std::string, Variant> variables;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
} // namespace plugkit
