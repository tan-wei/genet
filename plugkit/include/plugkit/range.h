/// @file
/// Range
#ifndef PLUGKIT_RANGE_H
#define PLUGKIT_RANGE_H

#include <stddef.h>

PLUGKIT_NAMESPACE_BEGIN

#define RANGE_NPOS ((size_t)-1)

typedef struct Range {
  size_t begin;
  size_t end;
} Range;

inline Range Range_offset(Range base, size_t offset) {
  Range range = {base.begin + offset, base.end + offset};
  return range;
}

PLUGKIT_NAMESPACE_END

#endif
