/// @file
#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include <cstdint>

extern "C" {

/// Global ID corresponded with a string
typedef uint32_t Token;

/// Returns a token corresponded with a given string.
///
/// @remarks This function is thread-safe.
Token Token_get(const char *str);

/// Returns the null token.
///
/// @code
/// Token_null() == Token_get(nullptr) == Token_get("")
/// @endcode
/// @remarks This function is thread-safe.
Token Token_null();

/// Returns a string corresponded with a given token.
///
/// @remarks This function is thread-safe.
const char *Token_string(Token token);
}

#endif
