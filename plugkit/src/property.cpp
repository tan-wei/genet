#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>

namespace plugkit {

class Property::Private {
public:
  Private(Token id, const Variant &value);
  Token id;
  Range range;
  Variant value;
  std::vector<const Property *> children;
};

Property::Private::Private(Token id, const Variant &value)
    : id(id), value(value) {}

Property::Property(Token id, const Variant &value)
    : d(new Private(id, value)) {}

Property::~Property() {}

Token Property::id() const { return d->id; }

void Property::setId(Token id) { d->id = id; }

Range Property::range() const { return d->range; }

void Property::setRange(const Range &range) { d->range = range; }

Variant Property::value() const { return d->value; }

void Property::setValue(const Variant &value) { d->value = value; }

const std::vector<const Property *> &Property::properties() const {
  return d->children;
}

const Property *Property::propertyFromId(Token id) const {
  for (const auto &child : d->children) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Property::addProperty(const Property *prop) {
  d->children.push_back(prop);
}
}
