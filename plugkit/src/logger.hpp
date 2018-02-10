#ifndef PLUGKIT_LOGGER_HPP
#define PLUGKIT_LOGGER_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <v8.h>

namespace plugkit {

struct Context;

class Logger {
public:
  enum Level {
    LEVEL_DEBUG = 0,
    LEVEL_INFO = 1,
    LEVEL_WARN = 2,
    LEVEL_ERROR = 3
  };

  struct Metadata {
    Level level;
    const char *file;
    uint32_t line;
    uint32_t column;
  };

  struct Message final {
    Level level = LEVEL_INFO;
    std::chrono::system_clock::time_point timestamp =
        std::chrono::system_clock::now();
    std::string message;
    std::string domain = "global";
    std::string resourceName;
    std::string sourceLine;
    int lineNumber = -1;
    int startPosition = -1;
    int endPosition = -1;
    int startColumn = -1;
    int endColumn = -1;
    std::thread::id threadId = std::this_thread::get_id();
    bool trivial = false;

    std::string toString() const;
    static const char *levelString(Level level);
  };

  using MessagePtr = std::unique_ptr<Message>;

public:
  virtual ~Logger();
  virtual void log(MessagePtr &&msg) = 0;
  virtual void log(Level level,
                   const std::string &message,
                   const std::string &domain = "global",
                   const std::string &resourceName = "");
  virtual void logTrivial(Level level,
                          const std::string &message,
                          const std::string &domain = "global",
                          const std::string &resourceName = "");

  static Logger::MessagePtr fromV8Message(v8::Local<v8::Message> msg,
                                          Logger::Level level,
                                          const std::string &domain = "global");

public:
  class Private;
};
using LoggerPtr = std::shared_ptr<Logger>;

extern "C" {

/// Return the ID of the layer.
void Logger_log(Context *ctx, const char *msg, const Logger::Metadata *meta);
}

} // namespace plugkit

#endif
