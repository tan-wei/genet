#ifndef PLUGKIT_ERROR_HPP
#define PLUGKIT_ERROR_HPP

#include "token.hpp"

namespace plugkit {

struct Error {
  Token id;
  Token target;
};

} // namespace plugkit

#endif
