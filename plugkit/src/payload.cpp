#include "payload.hpp"
#include "layer.hpp"
#include "property.hpp"

namespace plugkit {

class Payload::Private {
public:
  Private(const Slice &slice);

public:
  Slice slice;
  std::vector<const Property *> properties;
};

Payload::Private::Private(const Slice &slice) : slice(slice) {}

Payload::Payload(const Slice &slice) : d(new Private(slice)) {}

Payload::~Payload() {}

Slice Payload::slice() const { return d->slice; }

const std::vector<const Property *> &Payload::properties() const {
  return d->properties;
}

const Property *Payload::propertyFromId(Token id) const {
  for (const auto &prop : d->properties) {
    if (prop->id() == id) {
      return prop;
    }
  }
  return nullptr;
}

void Payload::addProperty(const Property *prop) {
  d->properties.push_back(prop);
}

View Payload_data(const Payload *payload) {
  const auto &slice = payload->slice();
  return View{slice.data(), slice.data() + slice.length()};
}
}
