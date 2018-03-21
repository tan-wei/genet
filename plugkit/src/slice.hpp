/// @file
/// Binary sequence
#ifndef PLUGKIT_SLICE_H
#define PLUGKIT_SLICE_H

#include "token.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace plugkit {

struct Slice {
  const char *data;
  size_t length;
};

} // namespace plugkit

#endif
