/// @file
/// Global ID corresponded with a string
#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include <stdint.h>
#include <stddef.h>
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

typedef uint32_t Token;

#define Token_get(str) Token_literal_(str, sizeof(str) - 1)
PLUGKIT_EXPORT Token Token_literal_(const char *str, size_t length);

/// Returns a token corresponded with a given string.
///
/// @remarks This function is thread-safe.
PLUGKIT_EXPORT Token Token_from(const char *str);

/// Returns the null token.
///
/// @code
/// Token_null() == Token_from(nullptr) == Token_get("")
/// @endcode
/// @remarks This function is thread-safe.
inline Token Token_null() { return (Token)0; }

/// Returns a string corresponded with a given token.
///
/// @remarks This function is thread-safe.
PLUGKIT_EXPORT const char *Token_string(Token token);

PLUGKIT_NAMESPACE_END

#endif
