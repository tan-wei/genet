#include "attribute.hpp"
#include "wrapper/attribute.hpp"
#include <iostream>

namespace plugkit {

Attr::Attr(Token id, const Variant &value, Token type)
    : mId(id), mValue(value), mType(type) {}

Attr::~Attr() {}

Token Attr::id() const { return mId; }

Range Attr::range() const { return mRange; }

void Attr::setRange(const Range &range) { mRange = range; }

Variant Attr::value() const { return mValue; }

const Variant *Attr::valueRef() const { return &mValue; }

Variant *Attr::valueRef() { return &mValue; }

void Attr::setValue(const Variant &value) { mValue = value; }

Token Attr::type() const { return mType; }

void Attr::setType(Token type) { mType = type; }

Token Attr_id(const Attr *prop) {
  if (!prop)
    return Token_null();
  return prop->id();
}

Range Attr_range(const Attr *prop) {
  if (!prop)
    return Range{0, 0};
  return prop->range();
}

void Attr_setRange(Attr *prop, Range range) { prop->setRange(range); }

Token Attr_type(const Attr *prop) {
  if (!prop)
    return Token_null();
  return prop->type();
}

void Attr_setType(Attr *prop, Token type) { prop->setType(type); }

const Variant *Attr_value(const Attr *prop) { return prop->valueRef(); }

Variant *Attr_valueRef(Attr *prop) { return prop->valueRef(); }

bool Attr_bool(const Attr *prop) {
  if (!prop)
    return false;
  return prop->value().boolValue();
}

void Attr_setBool(Attr *prop, bool value) { prop->setValue(value); }

int32_t Attr_int32(const Attr *prop) {
  if (!prop)
    return 0ll;
  return prop->value().int32Value();
}

void Attr_setInt32(Attr *prop, int32_t value) { prop->setValue(value); }

int64_t Attr_int64(const Attr *prop) {
  if (!prop)
    return 0ll;
  return prop->value().int64Value();
}

void Attr_setInt64(Attr *prop, int64_t value) { prop->setValue(value); }

uint32_t Attr_uint32(const Attr *prop) {
  if (!prop)
    return 0ull;
  return prop->value().uint32Value();
}

void Attr_setUint32(Attr *prop, uint32_t value) { prop->setValue(value); }

uint64_t Attr_uint64(const Attr *prop) {
  if (!prop)
    return 0ull;
  return prop->value().uint64Value();
}

void Attr_setUint64(Attr *prop, uint64_t value) { prop->setValue(value); }

double Attr_double(const Attr *prop) {
  if (!prop)
    return 0.0;
  return prop->value().doubleValue();
}

void Attr_setDouble(Attr *prop, double value) { prop->setValue(value); }

const char *Attr_string(const Attr *prop) {
  if (!prop)
    return "";
  return prop->value().string().c_str();
}

void Attr_setString(Attr *prop, const char *str) { prop->setValue(str); }

Slice Attr_slice(const Attr *prop) {
  if (!prop)
    return Slice{nullptr, nullptr};
  return prop->value().slice();
}

void Attr_setSlice(Attr *prop, Slice slice) { prop->setValue(slice); }

const Variant *Attr_arrayValue(const Attr *prop, size_t index) {
  if (!prop) {
    static const Variant null;
    return &null;
  }
  return Variant_arrayValue(prop->valueRef(), index);
}

const Variant *Attr_mapValue(const Attr *prop, const char *key, int length) {
  if (!prop) {
    static const Variant null;
    return &null;
  }
  return Variant_mapValue(prop->valueRef(), key, length);
}

Variant *Attr_mapValueRef(Attr *prop, const char *key, int length) {
  return Variant_mapValueRef(prop->valueRef(), key, length);
}
}
