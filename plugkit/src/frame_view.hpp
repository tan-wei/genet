#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include <memory>
#include <vector>
#include "token.h"
#include "property.hpp"

namespace plugkit {

class Frame;
class FrameView;
struct Layer;
struct Property;

class FrameView final {
public:
  explicit FrameView(Frame *frame);
  ~FrameView();
  const Frame *frame() const;
  const Layer *primaryLayer() const;
  const std::vector<const Layer *> &leafLayers() const;
  const Property *propertyFromId(Token id) const;
  const Layer *layerFromId(Token id) const;

private:
  FrameView(const FrameView &view) = delete;
  FrameView &operator=(const FrameView &view) = delete;

private:
  const Frame *mFrame;
  const Layer *mPrimaryLayer;
  std::vector<const Layer *> mLeafLayers;
  std::vector<const Layer *> mLayers;
};
}

#endif
