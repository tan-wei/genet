#include "private/stream_buffer.hpp"

#include <cstring>
#include <sys/stat.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#if defined(PLUGKIT_OS_LINUX)
#define MB_MMAP_FLAGS MAP_SHARED
#elif defined(PLUGKIT_OS_MAC)
#define MB_MMAP_FLAGS MAP_SHARED | MAP_NOCACHE
#endif

namespace plugkit {

StreamBuffer StreamBuffer::Private::create(uint64_t id) {
  return StreamBuffer(id);
}

StreamBuffer::StreamBuffer() : d(new Private()) {
  d->id = 0;

#ifdef PLUGKIT_OS_LINUX
  d->fd = open("/tmp", O_TMPFILE | O_RDWR, 0600);
#else
  char filename[] = "/tmp/mbuf.XXXXXX";
  d->fd = mkstemp(filename);
  unlink(filename);
#endif
  ftruncate(d->fd, d->length);
  d->begin = (char *)mmap(NULL, d->length, PROT_READ | PROT_WRITE,
                          MB_MMAP_FLAGS, d->fd, 0);
}

StreamBuffer::StreamBuffer(uint64_t id) : d(new Private()) { d->id = id; }

StreamBuffer::~StreamBuffer() {
  munmap(d->begin, d->length);
  close(d->fd);
}

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
