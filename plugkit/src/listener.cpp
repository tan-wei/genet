#include "listener.hpp"

namespace plugkit {

Listener::~Listener() {}

std::vector<AttributeConstPtr> Listener::properties() const {
  return std::vector<AttributeConstPtr>();
}

std::vector<ChunkConstPtr> Listener::chunks() const {
  return std::vector<ChunkConstPtr>();
}
}
