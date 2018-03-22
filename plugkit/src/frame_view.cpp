#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include <algorithm>
#include <functional>
#include <vector>

namespace plugkit {

FrameView::FrameView(Frame *frame) : mFrame(frame) {
  frame->setView(this);

  std::function<void(const Layer *)> findLeafLayers =
      [this, &findLeafLayers](const Layer *layer) {
        if (!layer)
          return;
        if (layer->layers().empty()) {
          mLeafLayers.push_back(layer);
        } else {
          for (const Layer *child : layer->layers()) {
            findLeafLayers(child);
          }
        }
      };
  findLeafLayers(mFrame->rootLayer());
}

FrameView::~FrameView() {}

const Frame *FrameView::frame() const { return mFrame; }

const Layer *FrameView::primaryLayer() const {
  if (mLeafLayers.empty()) {
    return nullptr;
  }
  return mLeafLayers.front();
}

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

double FrameView::timestamp() const {
  return static_cast<double>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          mFrame->timestamp().time_since_epoch())
          .count() /
      1000000.0);
}

Slice FrameView::payload() const {
  if (const Layer *root = mFrame->rootLayer()) {
    const auto &payloads = root->payloads();
    if (!payloads.empty()) {
      const auto &payload = payloads[0];
      if (!payload->slices().empty()) {
        return payload->slices()[0];
      }
    }
  }
  return Slice();
}

void FrameView::query(Token id, const Layer **layer, const Attr **attr) const {
  for (const Layer *leaf : leafLayers()) {
    for (const Layer *parent = leaf; parent; parent = parent->parent()) {
      if (parent->id() == id) {
        *layer = parent;
        return;
      }
      if (const Attr *layerAttr = parent->attr(id)) {
        *attr = layerAttr;
        return;
      }
    }
  }
}
} // namespace plugkit
