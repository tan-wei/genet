#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "property.hpp"
#include <functional>
#include <mutex>

namespace plugkit {

class FrameView::Private {
public:
  const Frame *frame;
  const Layer *primaryLayer;
  std::vector<const Layer *> leafLayers;
  std::vector<const Layer *> layers;
  bool hasError = false;
  std::mutex mutex;
};

FrameView::FrameView(const Frame *frame) : d(new Private()) {
  d->frame = frame; // FIXME

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
    if (d->primaryLayer->error().size()) {
      d->hasError = true;
    } else {
      for (const Property *prop : d->primaryLayer->properties()) {
        if (prop->error().size()) {
          d->hasError = true;
          break;
        }
      }
    }
  }

  propertyFromId(PK_STRID("src"));
  propertyFromId(PK_STRID("src"));
}

FrameView::~FrameView() {}

const Frame *FrameView::frame() const { return d->frame; }

const Layer *FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<const Layer *> &FrameView::leafLayers() const {
  return d->leafLayers;
}

const Property *FrameView::propertyFromId(strid id) const {
  for (auto layer = primaryLayer(); layer; layer = layer->parent()) {
    if (const auto &layerProp = layer->propertyFromId(id)) {
      return layerProp;
    }
  }
  return nullptr;
}

const Layer *FrameView::layerFromId(strid id) const {
  for (const auto &layer : d->layers) {
    if (layer->id() == id) {
      return layer;
    }
  }
  return nullptr;
}

bool FrameView::hasError() const { return d->hasError; }
}
