#ifndef PLUGKIT_FRAME_PRIVATE_H
#define PLUGKIT_FRAME_PRIVATE_H

#include "../frame.hpp"
#include <vector>

namespace plugkit {

class Frame;
class Layer;

class Frame::Private final {
public:
  Timestamp timestamp() const;
  void setTimestamp(const Timestamp &timestamp);
  size_t length() const;
  void setLength(size_t length);
  uint32_t index() const;
  void setIndex(uint32_t index);
  Layer *rootLayer() const;
  void setRootLayer(Layer *layer);

  uint32_t sourceId() const;
  void setSourceId(uint32_t id);

public:
  static Frame *create();

private:
  Timestamp timestamp_ = std::chrono::system_clock::now();
  size_t length_ = 0;
  uint32_t seq_ = 0;
  Layer *layer_ = nullptr;
  uint32_t sourceId_ = 0;
};
}

#endif
