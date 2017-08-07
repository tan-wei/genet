#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

Payload::Payload(const Slice &slice) : mData(slice) {}

Payload::~Payload() {}

Slice Payload::data() const { return mData; }

const std::vector<const Property *> &Payload::properties() const {
  return mProperties;
}

const Property *Payload::propertyFromId(Token id) const {
  for (const auto &prop : mProperties) {
    if (prop->id() == id) {
      return prop;
    }
  }
  return nullptr;
}

void Payload::addProperty(const Property *prop) { mProperties.push_back(prop); }

Slice Payload_data(const Payload *payload) { return payload->data(); }
}
