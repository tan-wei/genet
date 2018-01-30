#ifndef PLUGKIT_RANGE_H
#define PLUGKIT_RANGE_H

#include <stddef.h>

#define RANGE_NPOS ((size_t)-1)

namespace plugkit {

struct Range {
  size_t begin;
  size_t end;
};

inline Range Range_offset(Range base, size_t offset) {
  Range range = {base.begin + offset, base.end + offset};
  return range;
}

} // namespace plugkit

#endif
