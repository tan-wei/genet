#ifndef PLUGKIT_LOGGER_PRIVATE_H
#define PLUGKIT_LOGGER_PRIVATE_H

#include "../plugkit/logger.hpp"
#include <v8.h>

namespace plugkit {

class Logger::Private final {
public:
  static Logger::MessagePtr fromV8Message(v8::Local<v8::Message> msg,
                                          Logger::Level level,
                                          const std::string &domain = "global");
};
}

#endif
