#ifndef PLUGKIT_UVLOOP_LOGGER_HPP
#define PLUGKIT_UVLOOP_LOGGER_HPP

#include "logger.hpp"

struct uv_loop_s;
typedef struct uv_loop_s uv_loop_t;

namespace plugkit {

class UvLoopLogger final : public Logger {
public:
  using Callback = std::function<void(MessagePtr &&)>;
  UvLoopLogger(uv_loop_t *loop, const Callback &callback);
  ~UvLoopLogger();
  UvLoopLogger(const UvLoopLogger &) = delete;
  UvLoopLogger &operator=(const UvLoopLogger &) = delete;
  void log(MessagePtr &&msg) override;

private:
  class Private;
  Private *d;
};
}

#endif
