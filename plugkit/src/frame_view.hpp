#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include <memory>
#include <vector>
#include "strid.hpp"

namespace plugkit {

class Frame;
using FrameConstPtr = const Frame *;
using FrameUniquePtr = Frame *;

class FrameView;
using FrameViewConstPtr = const FrameView *;

class Layer;
using LayerConstPtr = const Layer *;

class Property;
using PropertyConstPtr = const Property *;

class FrameView final {
public:
  explicit FrameView(FrameConstPtr frame);
  ~FrameView();
  FrameConstPtr frame() const;
  LayerConstPtr primaryLayer() const;
  const std::vector<LayerConstPtr> &leafLayers() const;
  PropertyConstPtr propertyFromId(strid id) const;
  LayerConstPtr layerFromId(strid id) const;
  bool hasError() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
