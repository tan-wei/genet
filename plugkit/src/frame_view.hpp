#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include "attr.hpp"
#include "token.h"
#include <map>
#include <memory>
#include <vector>

namespace plugkit {

class Frame;
class FrameView;
struct Layer;
struct Attr;

class FrameView final {
public:
  explicit FrameView(Frame *frame);
  ~FrameView();
  const Frame *frame() const;
  const Layer *primaryLayer() const;
  const std::vector<const Layer *> &leafLayers() const;
  const Attr *attr(Token id) const;
  const Layer *layer(Token id) const;

  Layer *addModifiedLayer(const Layer *master);
  void clearModifiedLayers();

private:
  FrameView(const FrameView &view) = delete;
  FrameView &operator=(const FrameView &view) = delete;

private:
  const Frame *mFrame;
  const Layer *mPrimaryLayer;
  std::vector<const Layer *> mLeafLayers;
  std::vector<const Layer *> mLayers;
  std::map<const Layer *, Layer *> mModifiedLayers;
};
} // namespace plugkit

#endif
