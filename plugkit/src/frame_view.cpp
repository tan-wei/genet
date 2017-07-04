#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "property.hpp"
#include <functional>
#include <mutex>

namespace plugkit {

class FrameView::Private {
public:
  FrameConstPtr frame;
  LayerConstPtr primaryLayer;
  std::vector<LayerConstPtr> leafLayers;
  std::vector<LayerConstPtr> layers;
  bool hasError = false;
  std::mutex mutex;
};

FrameView::FrameView(FrameUniquePtr &&frame) : d(new Private()) {
  d->frame.reset(frame.release());

  std::function<void(const LayerConstPtr &)> findLeafLayers =
      [this, &findLeafLayers](const LayerConstPtr &layer) {
        d->layers.push_back(layer);
        if (layer->children().empty()) {
          d->leafLayers.push_back(layer);
        } else {
          for (const LayerConstPtr &child : layer->children()) {
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
      for (const PropertyConstPtr &prop : d->primaryLayer->properties()) {
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

FrameConstPtr FrameView::frame() const { return d->frame; }

LayerConstPtr FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<LayerConstPtr> &FrameView::leafLayers() const {
  return d->leafLayers;
}

PropertyConstPtr FrameView::propertyFromId(strid id) const {
  for (auto layer = primaryLayer(); layer; layer = layer->parent()) {
    if (const auto &layerProp = layer->propertyFromId(id)) {
      return layerProp;
    }
  }
  return PropertyConstPtr();
}

LayerConstPtr FrameView::layerFromId(strid id) const {
  for (const auto &layer : d->layers) {
    if (layer->id() == id) {
      return layer;
    }
  }
  return LayerConstPtr();
}

bool FrameView::hasError() const { return d->hasError; }
}
