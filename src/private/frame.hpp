#ifndef PLUGKIT_FRAME_PRIVATE_H
#define PLUGKIT_FRAME_PRIVATE_H

#include "../plugkit/frame.hpp"
#include <vector>

namespace plugkit {

class Frame;
using FrameUniquePtr = std::unique_ptr<Frame>;

class Layer;
using LayerPtr = std::shared_ptr<Layer>;

class Frame::Private final {
public:
  Timestamp timestamp() const;
  void setTimestamp(const Timestamp &timestamp);
  size_t length() const;
  void setLength(size_t length);
  uint32_t seq() const;
  void setSeq(uint32_t seq);
  LayerPtr rootLayer() const;
  void setRootLayer(const LayerPtr &layer);

public:
  static FrameUniquePtr create();

private:
  Timestamp timestamp_ = std::chrono::system_clock::now();
  size_t length_ = 0;
  uint32_t seq_ = 0;
  LayerPtr layer_;
};
}

#endif
