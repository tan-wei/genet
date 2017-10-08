#include "dissector.h"
#include <cstring>

namespace plugkit {
void Dissector_reset(Dissector *diss) {
  std::memset(diss, 0, sizeof(Dissector));
}
} // namespace plugkit
