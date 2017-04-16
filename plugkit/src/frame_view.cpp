#include "frame_view.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include <functional>
#include <algorithm>

namespace plugkit {

class FrameView::Private {
public:
  FrameConstPtr frame;
  LayerConstPtr primaryLayer;
  std::vector<LayerConstPtr> leafLayers;
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

  std::stable_sort(d->leafLayers.begin(), d->leafLayers.end(),
                   [](const LayerConstPtr &a, const LayerConstPtr &b) {
                     return b->confidence() < a->confidence();
                   });

  if (!d->leafLayers.empty()) {
    d->primaryLayer = d->leafLayers.front();
  }
}

FrameView::~FrameView() {}

FrameConstPtr FrameView::frame() const { return d->frame; }

LayerConstPtr FrameView::primaryLayer() const { return d->primaryLayer; }

const std::vector<LayerConstPtr> &FrameView::leafLayers() const {
  return d->leafLayers;
}
}
