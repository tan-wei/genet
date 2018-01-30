#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include <stdint.h>
#include <string.h>

namespace plugkit {

typedef uint32_t Token;

Token Token_literal_(const char *str, size_t length);
Token Token_join(Token prefix, Token token);

#ifdef PLUGKIT_OS_WIN
#pragma intrinsic(strlen)
#endif

/// Return a token corresponded with the given string.
inline Token Token_get(const char *str) {
  if (str == NULL || str[0] == '\0') {
    return (Token)0;
  }
  return Token_literal_(str, strlen(str));
}

#ifdef PLUGKIT_OS_WIN
#pragma function(strlen)
#endif

/// Return a string corresponded with the given token.
const char *Token_string(Token token);

} // namespace plugkit

#endif
