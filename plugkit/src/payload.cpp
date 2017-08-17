#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

Payload::Payload(const Slice &slice) : mData(slice), mType() {}

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

Token Payload::type() const { return mType; }

void Payload::setType(Token type) { mType = type; }

Slice Payload_data(const Payload *payload) { return payload->data(); }

Token Payload_type(const Payload *payload) { return payload->type(); }

void Payload_setType(Payload *payload, Token type) { payload->setType(type); }
}
