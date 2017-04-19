#ifndef PLUGKIT_STREAM_BUFFER_PRIVATE_H
#define PLUGKIT_STREAM_BUFFER_PRIVATE_H
#include "../plugkit/stream_buffer.hpp"
namespace plugkit {

class StreamBuffer::Private {
public:
  static StreamBuffer create(uint64_t id);

public:
  uint64_t id;
  size_t length = 0;

  int fd = 0;
  char *begin = 0;
};
}

#endif
