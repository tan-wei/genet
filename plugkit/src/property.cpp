#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>

namespace plugkit {

class Property::Private {
public:
  Private(strid id, const Variant &value);
  strid id;
  std::pair<uint32_t, uint32_t> range;
  std::string summary;
  std::string error;
  Variant value;
  std::vector<PropertyConstPtr> children;
};

Property::Private::Private(strid id, const Variant &value)
    : id(id), value(value) {}

Property::Property() : d(new Private(strid(), Variant())) {}

Property::Property(strid id, const Variant &value)
    : d(new Private(id, value)) {}

Property::~Property() {}

Property::Property(Property &&prop) { this->d.reset(prop.d.release()); }

strid Property::id() const { return d->id; }

void Property::setId(strid id) { d->id = id; }

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

const std::vector<PropertyConstPtr> &Property::properties() const {
  return d->children;
}

PropertyConstPtr Property::propertyFromId(strid id) const {
  for (const auto &child : d->children) {
    if (child->id() == id) {
      return child;
    }
  }
  return PropertyConstPtr();
}

void Property::addProperty(const PropertyConstPtr &prop) {
  d->children.push_back(prop);
}

void Property::addProperty(Property &&prop) {
  // TODO:ALLOC
  addProperty(new Property(std::move(prop)));
}
}
