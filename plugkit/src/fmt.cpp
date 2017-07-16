#include "fmt.hpp"
#include <string>

namespace plugkit {
namespace fmt {

template <> template <> uint8_t Reader<Slice>::readBE<uint8_t>() {
  return readLE<uint8_t>();
}

template <> template <> int8_t Reader<Slice>::readBE<int8_t>() {
  return readLE<int8_t>();
}
}
}
