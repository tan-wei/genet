#include "private/stream_buffer.hpp"

#include <cstring>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace plugkit {

std::atomic<uint64_t> StreamBuffer::Private::seq;

StreamBuffer StreamBuffer::Private::create(uint64_t id) {
  return StreamBuffer(id);
}

uint64_t StreamBuffer::Private::getSeq() {
  return seq.fetch_add(1u, std::memory_order_relaxed);
}

std::string StreamBuffer::Private::getTmpDir() {
  std::string path = "/tmp";
  const char *envs[] = {"TMP", "TEMP", "TMPDIR", "TEMPDIR"};
  for (const char *env : envs) {
    const char *tmp = std::getenv(env);
    if (tmp && strlen(tmp)) {
      path = tmp;
      break;
    }
  }
  path += "/plugkit_stream_";
#ifdef _WIN32
  //path += std::to_string(GetCurrentProcessId());
  //_mkdir(path.c_str());
#else
  path += std::to_string(getpid());
  mkdir(path.c_str(), 0755);
#endif
  return path;
}

StreamBuffer::StreamBuffer() : d(new Private()) { d->id = Private::getSeq(); }

StreamBuffer::StreamBuffer(uint64_t id) : d(new Private()) { d->id = id; }

StreamBuffer::~StreamBuffer() {}

StreamBuffer::StreamBuffer(const StreamBuffer &stream) {
  d.reset(new Private());
  d->id = stream.d->id;
}
StreamBuffer &StreamBuffer::operator=(const StreamBuffer &stream) {
  d.reset(new Private());
  d->id = stream.d->id;
  return *this;
}

size_t StreamBuffer::write(const char *data, size_t length) { return 0; }

size_t StreamBuffer::read(char *data, size_t max, size_t offset) const {
  return 0;
}

size_t StreamBuffer::length() const { return d->length; }

uint64_t StreamBuffer::id() const { return d->id; }
}
