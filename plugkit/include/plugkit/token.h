/// @file
/// Global ID corresponded with a string
#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include "export.h"
#include <stdint.h>
#include <string.h>

PLUGKIT_NAMESPACE_BEGIN

typedef uint32_t Token;

/// Return the null token.
inline Token Token_null() { return (Token)0; }

PLUGKIT_EXPORT Token Token_literal_(const char *str, size_t length);
PLUGKIT_EXPORT Token Token_join(Token prefix, Token token);

#ifdef PLUGKIT_OS_WIN
#pragma intrinsic(strlen)
#endif

/// Return a token corresponded with the given string.
inline Token Token_get(const char *str) {
  if (str == NULL || str[0] == '\0') {
    return Token_null();
  }
  return Token_literal_(str, strlen(str));
}

#ifdef PLUGKIT_OS_WIN
#pragma function(strlen)
#endif

/// Return a string corresponded with the given token.
PLUGKIT_EXPORT const char *Token_string(Token token);

PLUGKIT_NAMESPACE_END

#endif
