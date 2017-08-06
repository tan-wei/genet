#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

Payload::Payload(const View &view) : mView(view) {}

Payload::~Payload() {}

View Payload::view() const { return mView; }

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

View Payload_data(const Payload *payload) { return payload->view(); }
}
