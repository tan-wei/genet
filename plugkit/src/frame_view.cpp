#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "property.hpp"
#include <functional>
#include <unordered_map>

namespace plugkit {

class FrameView::Private {
public:
  FrameConstPtr frame;
  LayerConstPtr primaryLayer;
  std::vector<LayerConstPtr> leafLayers;
  std::unordered_map<std::string, PropertyConstPtr> properties;
  bool hasError = false;
};

FrameView::FrameView(FrameUniquePtr &&frame) : d(new Private()) {
  d->frame.reset(frame.release());

  std::function<void(const LayerConstPtr &)> findLeafLayers =
      [this, &findLeafLayers](const LayerConstPtr &layer) {
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

  propertyFromId("src");
  propertyFromId("dst");
}

FrameView::~FrameView() {}

FrameConstPtr FrameView::frame() const { return d->frame; }

LayerConstPtr FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<LayerConstPtr> &FrameView::leafLayers() const {
  return d->leafLayers;
}

PropertyConstPtr FrameView::propertyFromId(const std::string &id) const {
  const auto &it = d->properties.find(id);
  if (it != d->properties.end()) {
    return it->second;
  }

  PropertyConstPtr prop;
  for (auto layer = primaryLayer(); layer; layer = layer->parent()) {
    if (const auto &layerProp = layer->propertyFromId(id)) {
      prop = layerProp;
      break;
    }
  }
  d->properties[id] = prop;
  return prop;
}

bool FrameView::hasError() const { return d->hasError; }
}
