#ifndef PLUGKIT_PAYLOAD_HPP
#define PLUGKIT_PAYLOAD_HPP

#include "types.hpp"
#include "slice.hpp"
#include "token.h"
#include <memory>
#include <vector>

namespace plugkit {

class Payload {
public:
  Payload(const Slice &range);
  ~Payload();
  Slice slice() const;

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(Token id) const;
  void addProperty(const Property *prop);

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
