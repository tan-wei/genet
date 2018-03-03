#ifndef PLUGKIT_RANGE_H
#define PLUGKIT_RANGE_H

#include <cstdint>

#define RANGE_NPOS ((uint32_t)-1)

namespace plugkit {

struct Range {
  uint32_t begin;
  uint32_t end;
};

inline Range Range_offset(Range base, uint32_t offset) {
  Range range = {base.begin + offset, base.end + offset};
  return range;
}

} // namespace plugkit

#endif
