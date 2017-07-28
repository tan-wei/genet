#ifndef PLUGKIT_PAYLOAD_H
#define PLUGKIT_PAYLOAD_H

#include "types.hpp"
#include "miniid.hpp"
#include <vector>

namespace plugkit {

class Payload {
public:
  Payload(const Range &range, const Layer *layer = nullptr);
  ~Payload();

  const Layer *layer() const;
  Range range() const;

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(miniid id) const;
  void addProperty(const Property *prop);
};
}
