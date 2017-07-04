#include "private/chunk.hpp"
#include "property.hpp"
#include "slice.hpp"
#include "wrapper/chunk.hpp"

namespace plugkit {

Chunk::Private::Private(const std::string &ns, const std::string &id,
                        const Slice &payload)
    : streamNs(ns), streamId(id), payload(payload) {}

LayerConstPtr Chunk::Private::layer() const { return layer_.lock(); }

void Chunk::Private::setLayer(const LayerConstWeakPtr &layer) {
  layer_ = layer;
}

Chunk::Chunk() : d(new Private("", "", Slice())) {}

Chunk::Chunk(const std::string &ns, const std::string &id, const Slice &payload)
    : d(new Private(ns, id, payload)) {}

Chunk::Chunk(Chunk &&chunk) { this->d.reset(chunk.d.release()); }

Chunk::~Chunk() {}

std::string Chunk::streamNs() const { return d->streamNs; }

void Chunk::setStreamNs(const std::string &ns) { d->streamNs = ns; }

std::string Chunk::streamId() const { return d->streamId; }

void Chunk::setStreamId(const std::string &id) { d->streamId = id; }

const std::vector<PropertyConstPtr> &Chunk::properties() const {
  return d->properties;
}

const Slice &Chunk::payload() const { return d->payload; }

void Chunk::setPayload(const Slice &payload) { d->payload = payload; }

PropertyConstPtr Chunk::propertyFromId(strid id) const {
  for (const auto &child : d->properties) {
    if (child->id() == id) {
      return child;
    }
  }
  return PropertyConstPtr();
}

void Chunk::addProperty(const PropertyConstPtr &prop) {
  d->properties.push_back(prop);
}

LayerConstPtr Chunk::layer() const { return d->layer(); }
}
