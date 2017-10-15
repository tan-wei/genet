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

PLUGKIT_NAMESPACE_END

#endif
