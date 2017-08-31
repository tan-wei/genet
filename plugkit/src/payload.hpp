#ifndef PLUGKIT_PAYLOAD_HPP
#define PLUGKIT_PAYLOAD_HPP

#include "payload.h"
#include "types.hpp"
#include "slice.h"
#include "token.h"
#include <memory>
#include <vector>

namespace plugkit {

struct Payload final {
public:
  Payload();
  ~Payload();

  void addSlice(const Slice &slice);
  const std::vector<Slice> &slices() const;

  Token type() const;
  void setType(Token type);

  const std::vector<const Attr *> &properties() const;
  const Attr *attr(Token id) const;
  void addAttr(const Attr *prop);

private:
  Payload(const Payload &payload) = delete;
  Payload &operator=(const Payload &payload) = delete;

private:
  Token mType;
  std::vector<Slice> mSlices;
  std::vector<const Attr *> mProperties;
};
}

#endif
