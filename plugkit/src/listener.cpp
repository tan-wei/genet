#include "listener.hpp"

namespace plugkit {

std::vector<Property *> Listener::properties() const {
  return std::vector<Property *>();
}

std::vector<Chunk *> Listener::chunks() const { return std::vector<Chunk *>(); }
}
