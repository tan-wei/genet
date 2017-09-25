#include "stream_logger.hpp"

namespace plugkit {

StreamLogger::StreamLogger(std::ostream *stream) : stream(stream) {}

StreamLogger::~StreamLogger() {}

void StreamLogger::log(MessagePtr &&msg) {
  thread_local size_t previousHash = 0;
  if (msg->trivial) {
    size_t hash = std::hash<MessagePtr>{}(msg);
    if (hash == previousHash) {
      return;
    } else {
      previousHash = hash;
    }
  }
  *stream << msg->toString() << std::endl;
}
} // namespace plugkit
