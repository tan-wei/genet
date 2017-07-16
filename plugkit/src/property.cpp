#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>

namespace plugkit {

class Property::Private {
public:
  Private(miniid id, const Variant &value);
  miniid id;
  std::pair<uint32_t, uint32_t> range;
  std::string summary;
  std::string error;
  Variant value;
  std::vector<const Property *> children;
};

Property::Private::Private(miniid id, const Variant &value)
    : id(id), value(value) {}

Property::Property(miniid id, const Variant &value)
    : d(new Private(id, value)) {}

Property::~Property() {}

miniid Property::id() const { return d->id; }

void Property::setId(miniid id) { d->id = id; }

std::pair<uint32_t, uint32_t> Property::range() const { return d->range; }

void Property::setRange(const std::pair<uint32_t, uint32_t> &range) {
  d->range = range;
}

std::string Property::summary() const { return d->summary; }

void Property::setSummary(const std::string &summary) { d->summary = summary; }

std::string Property::error() const { return d->error; }

void Property::setError(const std::string &error) { d->error = error; }

Variant Property::value() const { return d->value; }

void Property::setValue(const Variant &value) { d->value = value; }

const std::vector<const Property *> &Property::properties() const {
  return d->children;
}

const Property *Property::propertyFromId(miniid id) const {
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
