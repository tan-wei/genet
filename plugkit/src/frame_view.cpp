#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include <vector>
#include <functional>

namespace plugkit {

namespace {
const Token lengthToken = Token_get(".length");
}

FrameView::FrameView(Frame *frame) : mFrame(frame), mPrimaryLayer(nullptr) {
  frame->setView(this);

  std::function<void(const Layer *)> findLeafLayers = [this, &findLeafLayers](
      const Layer *layer) {
    mLayers.push_back(layer);
    if (layer->children().empty()) {
      mLeafLayers.push_back(layer);
    } else {
      for (const Layer *child : layer->children()) {
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

const Property *FrameView::propertyFromId(Token id) const {
  for (const Layer *leaf : leafLayers()) {
    for (const Layer *layer = leaf; layer; layer = layer->parent()) {
      if (const Property *layerProp = layer->propertyFromId(id)) {
        return layerProp;
      }
    }
  }
  return nullptr;
}

const Layer *FrameView::layerFromId(Token id) const {
  for (const auto &layer : mLayers) {
    if (layer->id() == id) {
      return layer;
    }
  }
  return nullptr;
}
}
