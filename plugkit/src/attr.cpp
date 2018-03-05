#include "attr.hpp"
#include "wrapper/attr.hpp"
#include <iostream>

namespace plugkit {

Attr::Attr(Token id, const Variant &value, Token type)
    : mId(id), mType(type), mValue(value) {
  setRange({0, 0});
}

Attr::~Attr() {}

Token Attr::id() const { return mId; }

Range Attr::range() const { return mData.range; }

void Attr::setRange(const Range &range) { mData.range = range; }

Variant Attr::value() const { return mValue; }

const Variant *Attr::valueRef() const { return &mValue; }

Variant *Attr::valueRef() { return &mValue; }

void Attr::setValue(const Variant &value) { mValue = value; }

Token Attr::type() const { return mType; }

void Attr::setType(Token type) { mType = type; }

Token Attr_id(const Attr *attr) {
  if (!attr)
    return Token_null();
  return attr->id();
}

Range Attr_range(const Attr *attr) {
  if (!attr)
    return Range{0, 0};
  return attr->range();
}

void Attr_setRange(Attr *attr, Range range) { attr->setRange(range); }

Token Attr_type(const Attr *attr) {
  if (!attr)
    return Token_null();
  return attr->type();
}

void Attr_setType(Attr *attr, Token type) { attr->setType(type); }

const Variant *Attr_value(const Attr *attr) { return attr->valueRef(); }

Variant *Attr_valueRef(Attr *attr) { return attr->valueRef(); }

bool Attr_bool(const Attr *attr) {
  if (!attr)
    return false;
  return attr->value().boolValue();
}

void Attr_setBool(Attr *attr, bool value) { attr->setValue(value); }

int64_t Attr_int64(const Attr *attr) {
  if (!attr)
    return 0ll;
  return attr->value().int64Value();
}

void Attr_setInt64(Attr *attr, int64_t value) { attr->setValue(value); }

uint64_t Attr_uint64(const Attr *attr) {
  if (!attr)
    return 0ull;
  return attr->value().uint64Value();
}

void Attr_setUint64(Attr *attr, uint64_t value) { attr->setValue(value); }

double Attr_double(const Attr *attr) {
  if (!attr)
    return 0.0;
  return attr->value().doubleValue();
}

void Attr_setDouble(Attr *attr, double value) { attr->setValue(value); }

const char *Attr_string(const Attr *attr) {
  if (!attr)
    return "";
  return attr->value().string().c_str();
}

void Attr_setString(Attr *attr, const char *str) {
  Variant_setString(attr->valueRef(), str);
}

Slice Attr_slice(const Attr *attr) {
  if (!attr)
    return Slice{nullptr, 0};
  return attr->value().slice();
}

void Attr_setSlice(Attr *attr, Slice slice) { attr->setValue(slice); }

const Variant *Attr_arrayValue(const Attr *attr, size_t index) {
  if (!attr) {
    static const Variant null;
    return &null;
  }
  return Variant_arrayValue(attr->valueRef(), index);
}

Variant *Attr_arrayValueRef(Attr *attr, size_t index) {
  return Variant_arrayValueRef(attr->valueRef(), index);
}

const Variant *Attr_mapValue(const Attr *attr, const char *key, int length) {
  if (!attr) {
    static const Variant null;
    return &null;
  }
  return Variant_mapValue(attr->valueRef(), key, length);
}

Variant *Attr_mapValueRef(Attr *attr, const char *key, int length) {
  return Variant_mapValueRef(attr->valueRef(), key, length);
}
} // namespace plugkit
