#ifndef PLUGKIT_SESSION_CONTEXT_H
#define PLUGKIT_SESSION_CONTEXT_H

#include "variant.hpp"

namespace plugkit {

struct SessionContext {
  LoggerPtr logger;
  Variant options;
};
}

#endif
