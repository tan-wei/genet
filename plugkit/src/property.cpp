#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>

namespace plugkit {

Property::Property(Token id, const Variant &value, Token type)
    : mId(id), mValue(value), mType(type) {}

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

Token Property_id(const Property *prop) {
  if (!prop)
    return Token_null();
  return prop->id();
}

Range Property_range(const Property *prop) {
  if (!prop)
    return Range{0, 0};
  return prop->range();
}

void Property_setRange(Property *prop, Range range) { prop->setRange(range); }

Token Property_type(const Property *prop) {
  if (!prop)
    return Token_null();
  return prop->type();
}

void Property_setType(Property *prop, Token type) { prop->setType(type); }

const Variant *Property_value(const Property *prop) { return prop->valueRef(); }

Variant *Property_valueRef(Property *prop) { return prop->valueRef(); }

bool Property_bool(const Property *prop) {
  if (!prop)
    return false;
  return prop->value().boolValue();
}

void Property_setBool(Property *prop, bool value) { prop->setValue(value); }

int64_t Property_int64(const Property *prop) {
  if (!prop)
    return 0ll;
  return prop->value().int64Value();
}

void Property_setInt64(Property *prop, int64_t value) { prop->setValue(value); }

uint64_t Property_uint64(const Property *prop) {
  if (!prop)
    return 0ull;
  return prop->value().uint64Value();
}

void Property_setUint64(Property *prop, uint64_t value) {
  prop->setValue(value);
}

double Property_double(const Property *prop) {
  if (!prop)
    return 0.0;
  return prop->value().doubleValue();
}

void Property_setDouble(Property *prop, double value) { prop->setValue(value); }

const char *Property_string(const Property *prop) {
  if (!prop)
    return "";
  return prop->value().string().c_str();
}

void Property_setString(Property *prop, const char *str) {
  prop->setValue(str);
}

Slice Property_slice(const Property *prop) {
  if (!prop)
    return Slice{nullptr, nullptr};
  return prop->value().slice();
}

void Property_setSlice(Property *prop, Slice slice) { prop->setValue(slice); }

const Variant *Property_arrayValue(const Property *prop, size_t index) {
  if (!prop) {
    static const Variant null;
    return &null;
  }
  return Variant_arrayValue(prop->valueRef(), index);
}

const Variant *Property_mapValue(const Property *prop, const char *key,
                                 int length) {
  if (!prop) {
    static const Variant null;
    return &null;
  }
  return Variant_mapValue(prop->valueRef(), key, length);
}

Variant *Property_mapValueRef(Property *prop, const char *key, int length) {
  return Variant_mapValueRef(prop->valueRef(), key, length);
}
}
