#ifndef PLUGKIT_NULL_LOGGER_HPP
#define PLUGKIT_NULL_LOGGER_HPP

#include "logger.hpp"

namespace plugkit {

class NullLogger final : public Logger {
public:
  NullLogger();
  ~NullLogger();
  NullLogger(const NullLogger &) = delete;
  NullLogger &operator=(const NullLogger &) = delete;
  void log(MessagePtr &&msg) override;
  void log(Level level, const std::string &message,
           const std::string &domain = "global",
           const std::string &resourceName = "") override;
  void logTrivial(Level level, const std::string &message,
                  const std::string &domain = "global",
                  const std::string &resourceName = "") override;
};
} // namespace plugkit

#endif
