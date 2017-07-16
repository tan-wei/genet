#ifndef PLUGKIT_FRAME_H
#define PLUGKIT_FRAME_H

#include "slice.hpp"
#include "export.hpp"
#include "timestamp.hpp"
#include <memory>

namespace plugkit {

class Layer;

class PLUGKIT_EXPORT Frame final {
public:
  virtual ~Frame();
  Timestamp timestamp() const;
  size_t length() const;
  uint32_t index() const;
  Layer *rootLayer() const;

  uint32_t sourceId() const;

private:
  Frame();
  Frame(const Frame &) = delete;
  Frame &operator=(const Frame &) = delete;

public:
  friend class Private;
  class Private;
  Private *d;
};
}

#endif
