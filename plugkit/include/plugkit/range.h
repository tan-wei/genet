/// @file
/// Range
#ifndef PLUGKIT_RANGE_H
#define PLUGKIT_RANGE_H

#include <stdint.h>

PLUGKIT_NAMESPACE_BEGIN

typedef struct Range {
  uint64_t begin;
  uint64_t end;
} Range;

PLUGKIT_NAMESPACE_END

#endif
