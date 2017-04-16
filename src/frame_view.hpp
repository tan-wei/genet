#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include <memory>
#include <vector>

namespace plugkit {

class Frame;
using FrameConstPtr = std::shared_ptr<const Frame>;
using FrameUniquePtr = std::unique_ptr<Frame>;

class FrameView;
using FrameViewConstPtr = std::shared_ptr<const FrameView>;

class Layer;
using LayerConstPtr = std::shared_ptr<const Layer>;

class FrameView final {
public:
  explicit FrameView(FrameUniquePtr &&frame);
  ~FrameView();
  FrameConstPtr frame() const;
  LayerConstPtr primaryLayer() const;
  const std::vector<LayerConstPtr> &leafLayers() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
