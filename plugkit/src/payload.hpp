#ifndef PLUGKIT_PAYLOAD_HPP
#define PLUGKIT_PAYLOAD_HPP

#include "payload.h"
#include "range.h"
#include "slice.h"
#include "token.h"
#include "types.hpp"
#include <memory>
#include <vector>

namespace plugkit {

struct Payload final {
public:
  Payload();
  ~Payload();

  void addSlice(const Slice &slice);
  const std::vector<Slice> &slices() const;
  size_t length() const;

  Token type() const;
  void setType(Token type);

  Range range() const;
  void setRange(const Range &range);

  const std::vector<const Attr *> &attrs() const;
  const Attr *attr(Token id) const;
  void addAttr(const Attr *prop);

private:
  Payload(const Payload &payload) = delete;
  Payload &operator=(const Payload &payload) = delete;

private:
  Token mType;
  Range mRange = {0, 0};
  std::vector<Slice> mSlices;
  std::vector<const Attr *> mAttrs;
  size_t mLength = 0;
};
} // namespace plugkit

#endif
