#ifndef PLUGKIT_FRAME_H
#define PLUGKIT_FRAME_H

#include "slice.hpp"
#include "export.hpp"
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
  struct {
    Timestamp timestamp = std::chrono::system_clock::now();
    size_t length = 0;
    uint32_t seq = 0;
    Layer *layer = nullptr;
    uint32_t sourceId = 0;
  } data;
};
}

#endif
