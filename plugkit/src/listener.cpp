#include "listener.hpp"

namespace plugkit {

Listener::~Listener() {}

std::vector<AttributeConstPtr> Listener::attributes() const {
  return std::vector<AttributeConstPtr>();
}

std::vector<ChunkConstPtr> Listener::chunks() const {
  return std::vector<ChunkConstPtr>();
}
}
