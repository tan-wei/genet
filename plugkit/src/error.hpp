#ifndef PLUGKIT_ERROR_HPP
#define PLUGKIT_ERROR_HPP

#include "token.hpp"

namespace plugkit {

struct Error {
  Token id = Token_null();
  Token target = Token_null();
};

} // namespace plugkit
