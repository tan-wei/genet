#ifndef PLUGKIT_SESSION_CONTEXT_H
#define PLUGKIT_SESSION_CONTEXT_H

#include "variant.hpp"

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

struct SessionContext {
  LoggerPtr logger;
  Variant options;
};
}

#endif
