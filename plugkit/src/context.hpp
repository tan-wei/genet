#include "context.h"
#include "stream_logger.hpp"
#include "variant.hpp"
#include <unordered_map>

namespace plugkit {

using OptionMap = std::unordered_map<std::string, Variant>;

struct Context final {
public:
  OptionMap options;
  OptionMap variables;
  LoggerPtr logger = std::make_shared<StreamLogger>();
};
} // namespace plugkit
