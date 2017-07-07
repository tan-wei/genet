#ifndef PLUGKIT_CHUNK_PRIVATE_H
#define PLUGKIT_CHUNK_PRIVATE_H

#include "../plugkit/chunk.hpp"
#include "slice.hpp"
#include <unordered_map>

namespace plugkit {

class Layer;
using LayerConstWeakPtr = std::weak_ptr<const Layer>;

class Chunk::Private {
public:
  Private(const strns &ns, const std::string &id, const Slice &payload);

  LayerConstPtr layer() const;
  void setLayer(const LayerConstWeakPtr &layer);

public:
  strns streamNs;
  std::string streamId;
  Slice payload;
  std::vector<PropertyConstPtr> properties;
  std::unordered_map<std::string, size_t> idMap;
  LayerConstWeakPtr layer_;
};
}

#endif
