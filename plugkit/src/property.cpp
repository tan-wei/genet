#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>

namespace plugkit {

Property::Property(Token id, const Variant &value) : mId(id), mValue(value) {}

Property::~Property() {}

Token Property::id() const { return mId; }

Range Property::range() const { return mRange; }

void Property::setRange(const Range &range) { mRange = range; }

Variant Property::value() const { return mValue; }

const Variant *Property::valueRef() const { return &mValue; }

Variant *Property::valueRef() { return &mValue; }

void Property::setValue(const Variant &value) { mValue = value; }

Token Property::type() const { return mType; }

void Property::setType(Token type) { mType = type; }

const std::vector<const Property *> &Property::properties() const {
  return mChildren;
}

const Property *Property::propertyFromId(Token id) const {
  for (const auto &child : mChildren) {
    if (child->id() == id) {
      return child;
    }
  }
  return nullptr;
}

void Property::addProperty(const Property *prop) { mChildren.push_back(prop); }

Token Property_id(const Property *prop) { return prop->id(); }

Property *Property_addProperty(Property *prop, Token id) {
  Property *child = new Property(id);
  prop->addProperty(child);
  return child;
}

const Property *Property_propertyFromId(const Property *prop, Token id) {
  return prop->propertyFromId(id);
}

Range Property_range(const Property *prop) { return prop->range(); }

void Property_setRange(Property *prop, Range range) { prop->setRange(range); }

Token Property_type(const Property *prop) { return prop->type(); }

void Property_setType(Property *prop, Token type) { prop->setType(type); }

const Variant *Property_value(const Property *prop) { return prop->valueRef(); }

Variant *Property_valueRef(Property *prop) { return prop->valueRef(); }
}
