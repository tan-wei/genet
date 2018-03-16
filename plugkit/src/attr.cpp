#include "attr.hpp"
#include "wrapper/attr.hpp"
#include <iostream>

namespace plugkit {

Attr::Attr(Token id, const Variant &value, Token type)
    : mId(id), mType(type), mValue(value), mRange({0, 0}) {}

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

} // namespace plugkit
