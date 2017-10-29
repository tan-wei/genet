#include "payload.hpp"
#include "attr.hpp"
#include "layer.hpp"

namespace plugkit {

Payload::Payload() : mType() {}

Payload::~Payload() {}

void Payload::addSlice(const Slice &slice) {
  mSlices.push_back(slice);
  mLength += Slice_length(slice);
}

const std::vector<Slice> &Payload::slices() const { return mSlices; }

size_t Payload::length() const { return mLength; }

const std::vector<const Attr *> &Payload::attrs() const { return mAttrs; }

const Attr *Payload::attr(Token id) const {
  for (const auto &prop : mAttrs) {
    if (prop->id() == id) {
      return prop;
    }
  }
  return nullptr;
}

void Payload::addAttr(const Attr *prop) { mAttrs.push_back(prop); }

Token Payload::type() const { return mType; }

void Payload::setType(Token type) { mType = type; }

bool Payload::ref() const { return mRef; }

void Payload::setRef(bool ref) { mRef = ref; }

void Payload_addSlice(Payload *payload, Slice slice) {
  payload->addSlice(slice);
}

const Slice *Payload_slices(const Payload *payload, size_t *size) {
  const auto &slices = payload->slices();
  if (size)
    *size = slices.size();
  if (slices.empty()) {
    static const Slice nil = {nullptr, nullptr};
    return &nil;
  }
  return slices.data();
}

Token Payload_type(const Payload *payload) { return payload->type(); }

void Payload_setType(Payload *payload, Token type) { payload->setType(type); }

Attr *Payload_addAttr(Payload *payload, Token id) {
  Attr *prop = new Attr(id);
  payload->addAttr(prop);
  return prop;
}
} // namespace plugkit
