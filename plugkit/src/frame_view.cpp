#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "property.hpp"
#include <functional>
#include <algorithm>
#include <unordered_map>

namespace plugkit {

class FrameView::Private {
public:
  FrameConstPtr frame;
  LayerConstPtr primaryLayer;
  std::vector<LayerConstPtr> leafLayers;
  bool hasError = false;
};

FrameView::FrameView(FrameUniquePtr &&frame) : d(new Private()) {
  d->frame.reset(frame.release());

  std::unordered_map<const Layer *, double> confidenceMap;

  std::function<void(const LayerConstPtr &, double confidence)> findLeafLayers =
      [this, &findLeafLayers, &confidenceMap](const LayerConstPtr &layer,
                                              double confidence) {
        if (layer->children().empty()) {
          confidenceMap[layer.get()] = confidence * layer->confidence();
          d->leafLayers.push_back(layer);
        } else {
          for (const LayerConstPtr &child : layer->children()) {
            findLeafLayers(child, confidence * layer->confidence());
          }
        }
      };
  findLeafLayers(d->frame->rootLayer(), 1.0);

  std::stable_sort(
      d->leafLayers.begin(), d->leafLayers.end(),
      [&confidenceMap](const LayerConstPtr &a, const LayerConstPtr &b) {
        return confidenceMap[b.get()] < confidenceMap[a.get()];
      });

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
}

FrameView::~FrameView() {}

FrameConstPtr FrameView::frame() const { return d->frame; }

LayerConstPtr FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<LayerConstPtr> &FrameView::leafLayers() const {
  return d->leafLayers;
}

bool FrameView::hasError() const { return d->hasError; }
}
