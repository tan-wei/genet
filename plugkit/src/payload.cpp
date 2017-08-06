#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

Payload::Payload(const Slice &slice) : mSlice(slice) {}

Payload::~Payload() {}

Slice Payload::slice() const { return mSlice; }

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

View Payload_data(const Payload *payload) {
  const auto &slice = payload->slice();
  return View{slice.data(), slice.data() + slice.length()};
}
}
