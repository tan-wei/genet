#ifndef PLUGKIT_LOGGER_HPP
#define PLUGKIT_LOGGER_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include "export.hpp"

namespace plugkit {

class PLUGKIT_EXPORT Logger {
public:
  enum Level { LEVEL_DEBUG, LEVEL_INFO, LEVEL_WARN, LEVEL_ERROR };

  struct Message {
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
    int sessionId = -1;
    std::thread::id threadId = std::this_thread::get_id();
    bool trivial = false;

    std::string levelString() const;
    std::string toString() const;
  };

  using MessagePtr = std::unique_ptr<Message>;

public:
  virtual ~Logger();
  virtual void log(MessagePtr &&msg) = 0;
  virtual void log(Level level, const std::string &message,
                   const std::string &domain = "global",
                   const std::string &resourceName = "");
  virtual void logTrivial(Level level, const std::string &message,
                          const std::string &domain = "global",
                          const std::string &resourceName = "");

public:
  class Private;
};
using LoggerPtr = std::shared_ptr<Logger>;
}

#endif
