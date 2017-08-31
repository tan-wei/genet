#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

Payload::Payload() : mType() {}

Payload::~Payload() {}

void Payload::addSlice(const Slice &slice) { mSlices.push_back(slice); }

const std::vector<Slice> &Payload::slices() const { return mSlices; }

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

Token Payload::type() const { return mType; }

void Payload::setType(Token type) { mType = type; }

void Payload_addSlice(Payload *payload, Slice slice) {
  payload->addSlice(slice);
}

Slice Payload_slice(const Payload *payload) {
  const auto &slices = payload->slices();
  if (slices.size() > 0) {
    return slices.front();
  }
  return Slice{nullptr, nullptr};
}

const Slice *Payload_slices(const Payload *payload, size_t *size) {
  const auto &slices = payload->slices();
  if (size)
    *size = slices.size();
  return slices.data();
}

Token Payload_type(const Payload *payload) { return payload->type(); }

void Payload_setType(Payload *payload, Token type) { payload->setType(type); }

Property *Payload_addProperty(Payload *payload, Token id) {
  Property *prop = new Property(id);
  payload->addProperty(prop);
  return prop;
}
}
