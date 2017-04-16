#ifndef PLUGKIT_STREAM_BUFFER_H
#define PLUGKIT_STREAM_BUFFER_H

#include <memory>

namespace plugkit {
class StreamBuffer final {
public:
  StreamBuffer();
  ~StreamBuffer();
  StreamBuffer(const StreamBuffer &stream);
  StreamBuffer &operator=(const StreamBuffer &stream);
  size_t write(const char *data, size_t length);
  size_t read(char *data, size_t max, size_t offset = 0) const;
  size_t length() const;
  uint64_t id() const;

private:
  StreamBuffer(uint64_t id);

public:
  friend class Private;
  class Private;

private:
  std::unique_ptr<Private> d;
};
}

#endif
