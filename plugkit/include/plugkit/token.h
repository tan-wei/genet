/// @file
/// Global ID corresponded with a string
#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include "export.h"
#include <stdint.h>
#include <string.h>

PLUGKIT_NAMESPACE_BEGIN

typedef uint32_t Token;

/// Returns the null token.
///
/// @code
/// Token_null() == Token_get(nullptr) == Token_get("")
/// @endcode
/// @remarks This function is thread-safe.
inline Token Token_null() { return (Token)0; }

PLUGKIT_EXPORT Token Token_literal_(const char *str, size_t length);

#ifdef PLUGKIT_OS_WIN
#pragma intrinsic(strlen)
#endif

/// Returns a token corresponded with a given string.
///
/// @remarks This function is thread-safe.
inline Token Token_get(const char *str) {
  if (str == NULL || str[0] == '\0') {
    return Token_null();
  }
  return Token_literal_(str, strlen(str));
}

#ifdef PLUGKIT_OS_WIN
#pragma function(strlen)
#endif

/// Returns a string corresponded with a given token.
///
/// @remarks This function is thread-safe.
PLUGKIT_EXPORT const char *Token_string(Token token);

PLUGKIT_NAMESPACE_END

#endif
