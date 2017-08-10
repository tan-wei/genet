/// @file
/// Error
#ifndef PLUGKIT_ERROR_H
#define PLUGKIT_ERROR_H

#include "token.h"

PLUGKIT_NAMESPACE_BEGIN

struct Error {
  /// Predefined error type
  Token type;
};

PLUGKIT_NAMESPACE_END

#endif
