#include "error.hpp"

namespace plugkit {

const char *Error_message(const Error *err) { return err->message.c_str(); }

void Error_setMessage(Error *err, const char *msg) { err->message = msg; }

} // namespace plugkit
