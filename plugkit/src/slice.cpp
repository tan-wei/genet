#include "slice.hpp"

namespace plugkit {
class Slice::Private {
public:
  Private(const Buffer &buf, size_t offset, size_t length);

public:
  Buffer buf;
  size_t offset;
  size_t length;
};

Slice::Private::Private(const Buffer &buf, size_t offset, size_t length)
    : buf(buf), offset(offset), length(length) {}

Slice::Slice() : d(new Private(Buffer(), 0, 0)) {}

Slice::~Slice() {}

Slice::Slice(const Buffer &buf, size_t offset)
    : d(new Private(buf, offset, buf->size())) {}
Slice::Slice(const Buffer &buf, size_t offset, size_t length)
    : d(new Private(buf, offset, length)) {}
Slice::Slice(const char *data, size_t length)
    : d(new Private(std::make_shared<std::string>(data, length), 0, length)) {}

Slice::Slice(const Slice &slice) : d(new Private(*slice.d)) {}

Slice &Slice::operator=(const Slice &slice) {
  d.reset(new Private(*slice.d));
  return *this;
}

const char *Slice::data() const {
  if (d->buf && d->offset < d->buf->size())
    return &d->buf->at(d->offset);
  return nullptr;
}

size_t Slice::length() const { return d->length; }

char Slice::operator[](size_t index) const {
  return (*d->buf)[d->offset + index];
}

Slice::Buffer Slice::buffer() const { return d->buf; }

size_t Slice::offset() const { return d->offset; }

Slice Slice::slice(size_t offset, size_t length) const {
  if (!d->buf)
    return Slice();
  auto clip = [](size_t value, size_t max) {
    return value > max ? max : value;
  };
  size_t s = this->length();
  size_t newOffset = clip(d->offset + offset, d->offset + s);
  size_t newLength = clip(clip(length, d->buf->size() - newOffset), s);
  return Slice(d->buf, newOffset, newLength);
}

Slice Slice::slice(size_t offset) const {
  return slice(offset, length() - offset);
}
}
