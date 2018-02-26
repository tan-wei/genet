#ifndef PLUGKIT_ERROR_HPP
#define PLUGKIT_ERROR_HPP

#include "token.hpp"
#include <string>

namespace plugkit {

struct Error {
  Token id;
  Token target;
  std::string message;
};

extern "C" {
const char *Error_message(const Error *err);
void Error_setMessage(Error *err, const char *msg);
}

} // namespace plugkit

#endif
