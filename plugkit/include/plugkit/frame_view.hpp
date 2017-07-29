#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include <memory>
#include <vector>
#include "miniid.hpp"
#include "export.hpp"

namespace plugkit {

class Frame;
class FrameView;
class Layer;
class Property;

class PLUGKIT_EXPORT FrameView final {
public:
  explicit FrameView(const Frame *frame);
  ~FrameView();
  const Frame *frame() const;
  const Layer *primaryLayer() const;
  const std::vector<const Layer *> &leafLayers() const;
  const Property *propertyFromId(miniid id) const;
  const Layer *layerFromId(miniid id) const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
