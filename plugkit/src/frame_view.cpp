#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include <algorithm>
#include <functional>
#include <vector>

namespace plugkit {

FrameView::FrameView(Frame *frame) : mFrame(frame), mPrimaryLayer(nullptr) {
  frame->setView(this);

  std::function<void(const Layer *)> findLeafLayers =
      [this, &findLeafLayers](const Layer *layer) {
        if (!layer)
          return;
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

  std::sort(mLeafLayers.begin(), mLeafLayers.end(),
            [](const Layer *a, const Layer *b) {
              if (a->confidence() == b->confidence()) {
                return a->id() < b->id();
              }
              return a->confidence() > b->confidence();
            });

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

Layer *FrameView::addModifiedLayer(const Layer *master) {
  Layer *modifed = new Layer(master->id());
  modifed->setMaster(master);
  mModifiedLayers[master] = modifed;
  return modifed;
}

void FrameView::clearModifiedLayers() {
  for (const auto &pair : mModifiedLayers) {
    delete pair.second;
  }
  mModifiedLayers.clear();
}

} // namespace plugkit
