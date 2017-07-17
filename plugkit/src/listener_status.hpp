#ifndef PLUGKIT_LISTENER_STATUS_H
#define PLUGKIT_LISTENER_STATUS_H

#include <memory>
#include <map>
#include <list>
#include "miniid.hpp"

namespace plugkit {

class Chunk;
class Property;

class ListenerStatus {
public:
  ListenerStatus();
  ~ListenerStatus();

  size_t properties() const;
  size_t chunks() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
