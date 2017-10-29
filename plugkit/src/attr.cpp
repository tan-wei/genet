#include "attr.hpp"
#include "wrapper/attr.hpp"
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

Token Attr::error() const { return mError; }

void Attr::setError(Token error) { mError = error; }

bool Attr::ref() const { return mRef; }

void Attr::setRef(bool ref) { mRef = ref; }

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

Token Attr_error(const Attr *attr) { return attr->error(); }

void Attr_setError(Attr *attr, Token error) { attr->setError(error); }

const Variant *Attr_value(const Attr *attr) { return attr->valueRef(); }

Variant *Attr_valueRef(Attr *attr) { return attr->valueRef(); }

bool Attr_bool(const Attr *attr) {
  if (!attr)
    return false;
  return attr->value().boolValue();
}

void Attr_setBool(Attr *attr, bool value) { attr->setValue(value); }

int32_t Attr_int32(const Attr *attr) {
  if (!attr)
    return 0ll;
  return attr->value().int32Value();
}

void Attr_setInt32(Attr *attr, int32_t value) { attr->setValue(value); }

uint32_t Attr_uint32(const Attr *attr) {
  if (!attr)
    return 0ull;
  return attr->value().uint32Value();
}

void Attr_setUint32(Attr *attr, uint32_t value) { attr->setValue(value); }

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

void Attr_setString(Attr *attr, const char *str) { attr->setValue(str); }

Slice Attr_slice(const Attr *attr) {
  if (!attr)
    return Slice{nullptr, nullptr};
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
