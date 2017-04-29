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

template <> template <> uint8_t Reader<Slice>::readBE<uint8_t>() {
  return readLE<uint8_t>();
}

template <> template <> int8_t Reader<Slice>::readBE<int8_t>() {
  return readLE<int8_t>();
}
}
}
