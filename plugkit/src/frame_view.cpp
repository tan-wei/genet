#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "property.hpp"
#include <vector>
#include <functional>

namespace plugkit {

class FrameView::Private {
public:
  Private(const Frame *frame);

public:
  const Frame *frame = nullptr;
  const Layer *primaryLayer = nullptr;
  std::vector<const Layer *> leafLayers;
  std::vector<const Layer *> layers;
};

FrameView::Private::Private(const Frame *frame) : frame(frame) {}

FrameView::FrameView(Frame *frame) : d(new Private(frame)) {
  frame->setView(this);

  std::function<void(const Layer *)> findLeafLayers = [this, &findLeafLayers](
      const Layer *layer) {
    d->layers.push_back(layer);
    if (layer->children().empty()) {
      d->leafLayers.push_back(layer);
    } else {
      for (const Layer *child : layer->children()) {
        findLeafLayers(child);
      }
    }
  };
  findLeafLayers(d->frame->rootLayer());

  if (!d->leafLayers.empty()) {
    d->primaryLayer = d->leafLayers.front();
  }
}

FrameView::~FrameView() {}

const Frame *FrameView::frame() const { return d->frame; }

const Layer *FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<const Layer *> &FrameView::leafLayers() const {
  return d->leafLayers;
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
  for (const auto &layer : d->layers) {
    if (layer->id() == id) {
      return layer;
    }
  }
  return nullptr;
}
}
