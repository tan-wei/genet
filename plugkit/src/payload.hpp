#ifndef PLUGKIT_PAYLOAD_HPP
#define PLUGKIT_PAYLOAD_HPP

#include "range.hpp"
#include "slice.hpp"
#include "token.hpp"
#include "types.hpp"
#include <memory>
#include <vector>

namespace plugkit {

struct Context;

struct Payload final {
public:
  Payload();
  ~Payload();

  void addSlice(const Slice &slice);
  const std::vector<Slice> &slices() const;
  uint32_t length() const;

  Token type() const;
  void setType(Token type);

  Range range() const;
  void setRange(const Range &range);

  const std::vector<const Attr *> &attrs() const;
  const Attr *attr(Token id) const;
  void addAttr(const Attr *attr);

private:
  Payload(const Payload &payload) = delete;
  Payload &operator=(const Payload &payload) = delete;

private:
  Token mType = Token_null();
  uint32_t mLength = 0;
  Range mRange = {0, 0};
  std::vector<Slice> mSlices;
  std::vector<const Attr *> mAttrs;
};

extern "C" {

void Payload_addSlice(Payload *payload, Slice slice);

/// Returns the first address of slices
/// and assigns the number of slices to size.
/// Returns the address of an empty slice if the payload has no slices.
const Slice *Payload_slices(const Payload *payload, size_t *size);

/// Allocates a new Attr and adds it as a payload attribute.
Attr *Payload_addAttr(Payload *payload, Context *ctx, Token id);
}

} // namespace plugkit

#endif
