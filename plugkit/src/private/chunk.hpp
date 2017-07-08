#ifndef PLUGKIT_CHUNK_PRIVATE_H
#define PLUGKIT_CHUNK_PRIVATE_H

#include "../plugkit/chunk.hpp"
#include "slice.hpp"
#include <unordered_map>

namespace plugkit {

class Layer;

class Chunk::Private {
public:
  Private(const strns &ns, const std::string &id, const Slice &payload);

  const Layer *layer() const;
  void setLayer(const Layer *layer);

public:
  strns streamNs;
  std::string streamId;
  Slice payload;
  std::vector<const Property *> properties;
  std::unordered_map<std::string, size_t> idMap;
  const Layer *layer_ = nullptr;
};
}

#endif
