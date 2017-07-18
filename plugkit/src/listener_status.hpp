#ifndef PLUGKIT_LISTENER_STATUS_H
#define PLUGKIT_LISTENER_STATUS_H

#include <memory>
#include <map>
#include <list>
#include "miniid.hpp"

namespace plugkit {

class Attribute;
using AttributeConstPtr = std::shared_ptr<const Attribute>;

class Chunk;
using ChunkConstPtr = std::shared_ptr<const Chunk>;

class ListenerStatus {
public:
  ListenerStatus();
  ~ListenerStatus();

  size_t attributes() const;
  size_t chunks() const;

  AttributeConstPtr getAttribute(size_t index) const;
  ChunkConstPtr getChunk(size_t index) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
