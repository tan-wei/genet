#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>
#include <unordered_map>

namespace plugkit {

class Property::Private {
public:
  Private(const char *id, const std::string &name, const Variant &value);
  char id[8];
  std::string name;
  std::pair<uint32_t, uint32_t> range;
  std::string summary;
  std::string error;
  Variant value;
  std::vector<PropertyConstPtr> children;
};

Property::Private::Private(const char *id, const std::string &name,
                           const Variant &value)
    : name(name), value(value) {
  std::memset(this->id, '\0', sizeof(this->id));
  std::strncpy(this->id, id, sizeof(this->id));
}

Property::Property() : d(new Private("", "", Variant())) {}

Property::Property(const char *id, const std::string &name,
                   const Variant &value)
    : d(new Private(id, name, value)) {}

Property::~Property() {}

Property::Property(Property &&prop) { this->d.reset(prop.d.release()); }

std::string Property::name() const { return d->name; }

void Property::setName(const std::string &name) { d->name = name; }

const char *Property::id() const { return d->id; }

void Property::setId(const char *id) {
  std::memset(d->id, '\0', sizeof(d->id));
  std::strncpy(d->id, id, sizeof(d->id));
}

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

PropertyConstPtr Property::propertyFromId(const char *id) const {
  char idbuf[8] = {0};
  for (int i = 0; i < 8 && id[i] != '\0'; ++i) {
    idbuf[i] = id[i];
  }
  for (const auto &child : d->children) {
    if (*reinterpret_cast<const uint64_t *>(child->id()) ==
        *reinterpret_cast<const uint64_t *>(idbuf)) {
      return child;
    }
  }
  return PropertyConstPtr();
}

void Property::addProperty(const PropertyConstPtr &prop) {
  d->children.push_back(prop);
}

void Property::addProperty(Property &&prop) {
  addProperty(std::make_shared<Property>(std::move(prop)));
}
}
