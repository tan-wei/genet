#include "chunk.hpp"
#include "wrapper/chunk.hpp"
#include "property.hpp"
#include "slice.hpp"
#include <unordered_map>

namespace plugkit {

class Chunk::Private {
public:
  Private() = default;
  std::string streamNs;
  std::string streamId;
  Slice payload;
  std::vector<PropertyConstPtr> properties;
  std::unordered_map<std::string, size_t> idMap;
};

Chunk::Chunk() : d(new Private()) {}

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

PropertyConstPtr Chunk::propertyFromId(const std::string &id) const {
  auto it = d->idMap.find(id);
  if (it != d->idMap.end()) {
    return d->properties[it->second];
  } else {
    return PropertyConstPtr();
  }
}

void Chunk::addProperty(const PropertyConstPtr &prop) {
  d->idMap[prop->id()] = d->properties.size();
  d->properties.push_back(prop);
}
}
