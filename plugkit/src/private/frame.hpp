#ifndef PLUGKIT_FRAME_PRIVATE_H
#define PLUGKIT_FRAME_PRIVATE_H

#include "../plugkit/frame.hpp"
#include <vector>

namespace plugkit {

class Frame;
using FrameUniquePtr = Frame *;

class Layer;
using LayerPtr = Layer *;

class Frame::Private final {
public:
  Timestamp timestamp() const;
  void setTimestamp(const Timestamp &timestamp);
  size_t length() const;
  void setLength(size_t length);
  uint32_t index() const;
  void setIndex(uint32_t index);
  LayerPtr rootLayer() const;
  void setRootLayer(const LayerPtr &layer);

  uint32_t sourceId() const;
  void setSourceId(uint32_t id);

public:
  static FrameUniquePtr create();

private:
  Timestamp timestamp_ = std::chrono::system_clock::now();
  size_t length_ = 0;
  uint32_t seq_ = 0;
  LayerPtr layer_;
  uint32_t sourceId_ = 0;
};
}

#endif
