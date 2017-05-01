#ifndef PLUGKIT_STREAM_BUFFER_PRIVATE_H
#define PLUGKIT_STREAM_BUFFER_PRIVATE_H
#include <atomic>
#include <fstream>
#include "../plugkit/stream_buffer.hpp"
namespace plugkit {

class StreamBuffer::Private {
public:
  static StreamBuffer create(uint64_t id);
  static uint64_t getSeq();
  static std::string getTmpDir();

public:
  uint64_t id;
  size_t length = 0;
  std::fstream stream;
  static std::atomic<uint64_t> index;
};
}

#endif
