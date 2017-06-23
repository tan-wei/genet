#ifndef PLUGKIT_FRAME_H
#define PLUGKIT_FRAME_H

#include "slice.hpp"
#include "export.hpp"
#include <chrono>
#include <memory>

namespace plugkit {

class Layer;
using LayerPtr = std::shared_ptr<Layer>;

class PLUGKIT_EXPORT Frame final {
public:
  using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                            std::chrono::nanoseconds>;

public:
  virtual ~Frame();
  Timestamp timestamp() const;
  size_t length() const;
  uint32_t index() const;
  LayerPtr rootLayer() const;

  uint32_t sourceId() const;

private:
  Frame();
  Frame(const Frame &) = delete;
  Frame &operator=(const Frame &) = delete;

public:
  friend class Private;
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
