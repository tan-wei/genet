#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include <functional>
#include <vector>

namespace plugkit {

namespace {
const Token lengthToken = Token_get(".length");
}

FrameView::FrameView(Frame *frame) : mFrame(frame), mPrimaryLayer(nullptr) {
  frame->setView(this);

  std::function<void(const Layer *)> findLeafLayers = [this, &findLeafLayers](
      const Layer *layer) {
    mLayers.push_back(layer);
    if (layer->layers().empty()) {
      mLeafLayers.push_back(layer);
    } else {
      for (const Layer *child : layer->layers()) {
        findLeafLayers(child);
      }
    }
  };
  findLeafLayers(mFrame->rootLayer());

  if (!mLeafLayers.empty()) {
    mPrimaryLayer = mLeafLayers.front();
  }
}

FrameView::~FrameView() {}

const Frame *FrameView::frame() const { return mFrame; }

const Layer *FrameView::primaryLayer() const { return mPrimaryLayer; }

const std::vector<const Layer *> &FrameView::leafLayers() const {
  return mLeafLayers;
}

const Attr *FrameView::attr(Token id) const {
  for (const Layer *leaf : leafLayers()) {
    for (const Layer *layer = leaf; layer; layer = layer->parent()) {
      if (const Attr *layerProp = layer->attr(id)) {
        return layerProp;
      }
    }
  }
  return nullptr;
}

const Layer *FrameView::layer(Token id) const {
  for (const auto &layer : mLayers) {
    if (layer->id() == id) {
      return layer;
    }
  }
  return nullptr;
}
} // namespace plugkit
