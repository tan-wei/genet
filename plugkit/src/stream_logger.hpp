#ifndef PLUGKIT_STDOUT_LOGGER_HPP
#define PLUGKIT_STDOUT_LOGGER_HPP

#include "logger.hpp"
#include <iostream>

namespace plugkit {

class StreamLogger final : public Logger {
public:
  StreamLogger(std::ostream *stream = &std::cout);
  ~StreamLogger();
  StreamLogger(const StreamLogger &) = delete;
  StreamLogger &operator=(const StreamLogger &) = delete;
  void log(MessagePtr &&msg) override;

private:
  std::ostream *stream;
};
}

#endif
