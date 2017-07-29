#ifndef PLUGKIT_TOKEN_H
#define PLUGKIT_TOKEN_H

#include <cstdint>

extern "C" {

typedef uint32_t Token;

Token Token_get(const char *token);
Token Token_null();
}

#endif
