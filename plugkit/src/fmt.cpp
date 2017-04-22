#include "fmt.hpp"
#include <string>

namespace plugkit {
namespace fmt {

std::string replace(const std::string &target, const std::string &before,
                    const std::string &after) {
  std::string result = target;
  auto pos = target.rfind(before);
  if (pos != std::string::npos) {
    result.replace(pos, before.size(), after);
  }
  return result;
}
}
}
