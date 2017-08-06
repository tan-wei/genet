#ifndef PLUGKIT_FRAME_H
#define PLUGKIT_FRAME_H

#include "export.h"
#include "types.hpp"
#include <memory>

namespace plugkit {

class PLUGKIT_EXPORT Frame final {
public:
  Frame();
  virtual ~Frame();
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

private:
  Frame(const Frame &) = delete;
  Frame &operator=(const Frame &) = delete;

private:
  Timestamp mTimestamp = std::chrono::system_clock::now();
  size_t mLength = 0;
  uint32_t mSeq = 0;
  Layer *mLayer = nullptr;
  uint32_t mSourceId = 0;
};
}

#endif
