#include "stream_dissector.hpp"
#include <chrono>

namespace plugkit {

StreamDissector::~StreamDissector() {}

class StreamDissector::Worker::Private {
public:
};

StreamDissector::Worker::Worker() : d(new Private()) {}

StreamDissector::Worker::~Worker() {}

bool StreamDissector::Worker::expired(const Timestamp &lastUpdated) const {
  return (std::chrono::system_clock::now() - lastUpdated) >
         std::chrono::minutes(10);
}

StreamDissectorFactory::~StreamDissectorFactory() {}
}
