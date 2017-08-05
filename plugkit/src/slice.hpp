#ifndef PLUGKIT_SLICE_H
#define PLUGKIT_SLICE_H

#include <memory>
#include <string>
#include "export.h"

namespace plugkit {

class PLUGKIT_EXPORT Slice final {
public:
  using Buffer = std::shared_ptr<const std::string>;

public:
  Slice();
  ~Slice();
  Slice(const char *data, size_t length);
  Slice(const Slice &slice);
  Slice &operator=(const Slice &slice);
  const char *data() const;
  size_t length() const;
  size_t offset() const;
  char operator[](size_t index) const;
  Slice slice(size_t offset, size_t length) const;
  Slice slice(size_t offset) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
