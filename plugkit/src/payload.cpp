#include "payload.hpp"
#include "attr.hpp"
#include "context.hpp"
#include "layer.hpp"

namespace plugkit {

Payload::Payload() : mType() {}

Payload::~Payload() {
  for (const auto &attr : mAttrs) {
    delete attr;
  }
}

void Payload::addSlice(const Slice &slice) {
  mSlices.push_back(slice);
  mLength += slice.length;
}

const std::vector<Slice> &Payload::slices() const { return mSlices; }

uint32_t Payload::length() const { return mLength; }

const std::vector<const Attr *> &Payload::attrs() const { return mAttrs; }

const Attr *Payload::attr(Token id) const {
  for (const auto &attr : mAttrs) {
    if (attr->id() == id) {
      return attr;
    }
  }
  return nullptr;
}

void Payload::addAttr(const Attr *attr) { mAttrs.push_back(attr); }

Token Payload::type() const { return mType; }

void Payload::setType(Token type) { mType = type; }

Range Payload::range() const { return mRange; }

void Payload::setRange(const Range &range) { mRange = range; }

void Payload_addSlice(Payload *payload, Slice slice) {
  payload->addSlice(slice);
}

const Slice *Payload_slices(const Payload *payload, size_t *size) {
  const auto &slices = payload->slices();
  if (size)
    *size = slices.size();
  if (slices.empty()) {
    static const Slice nil = {nullptr, 0};
    return &nil;
  }
  return slices.data();
}

Attr *Payload_addAttr(Payload *payload, Context *ctx, Token id) {
  Attr *attr = Context_allocAttr(ctx, id);
  payload->addAttr(attr);
  return attr;
}
} // namespace plugkit
