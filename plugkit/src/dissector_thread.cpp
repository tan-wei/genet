#include "dissector_thread.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "stream_resolver.hpp"
#include "variant.hpp"

#include "dissector.h"
#include "context.hpp"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstring>
#include <v8.h>

namespace plugkit {

namespace {

class TagFilter {
public:
  TagFilter();
  TagFilter(const Token *begin, size_t size);
  bool match(const Token *begin, size_t size) const;

private:
  uint64_t bloom = 0;
  std::vector<Token> filters;
};

TagFilter::TagFilter() {}

TagFilter::TagFilter(const Token *begin, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    if (begin[i] != Token_null()) {
      filters.push_back(begin[i]);
      bloom |= (0x1 << (begin[i] % 64));
    }
  }
}

bool TagFilter::match(const Token *begin, size_t size) const {
  uint64_t hash = 0;
  for (size_t i = 0; i < size; ++i) {
    hash |= (0x1 << (begin[i] % 64));
  }
  if ((hash & bloom) != bloom) {
    return false;
  }
  for (Token t : filters) {
    bool found = false;
    for (size_t i = 0; i < size; ++i) {
      if (t == begin[i]) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}

struct WorkerData {
  const Dissector *dissector;
  TagFilter filter;
  Worker *worker;
};
}

class DissectorThread::Private {
public:
  FrameQueuePtr queue;
  StreamResolverPtr resolver;
  Callback callback;
  std::vector<Dissector> dissectors;
  std::vector<WorkerData> workers;
  Variant options;
  double confidenceThreshold;
};

DissectorThread::DissectorThread(const Variant &options,
                                 const FrameQueuePtr &queue,
                                 const StreamResolverPtr &resolver,
                                 const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->queue = queue;
  d->resolver = resolver;
  d->callback = callback;
  d->confidenceThreshold =
      options["_"]["confidenceThreshold"].uint64Value(0) / 100.0;
}

DissectorThread::~DissectorThread() {}

void DissectorThread::pushDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void DissectorThread::enter() {
  Context ctx;
  for (const auto &diss : d->dissectors) {
    WorkerData data;
    data.dissector = &diss;
    data.filter = TagFilter(data.dissector->layerHints,
                            sizeof(data.dissector->layerHints) /
                                sizeof(data.dissector->layerHints[0]));
    if (diss.createWorker) {
      data.worker = diss.createWorker(&ctx);
    }
    d->workers.push_back(data);
  }
}

bool DissectorThread::loop() {
  std::array<Frame *, 128> frames;
  size_t size = d->queue->dequeue(std::begin(frames), frames.size());
  if (size == 0)
    return false;

  std::vector<std::pair<Layer *, std::string>> streamedLayers;

  for (size_t i = 0; i < size; ++i) {
    std::unordered_set<Token> dissectedIds;

    Frame *frame = frames[i];
    const auto &rootLayer = frame->rootLayer();
    if (!rootLayer)
      continue;

    std::vector<const WorkerData *> workers;
    std::vector<Layer *> leafLayers = {rootLayer};
    while (!leafLayers.empty()) {
      std::vector<Layer *> nextlayers;
      for (const auto &layer : leafLayers) {
        dissectedIds.insert(layer->id());

        const auto &tags = layer->tags();

        workers.clear();
        for (const auto &data : d->workers) {
          if (data.filter.match(tags.data(), tags.size())) {
            workers.push_back(&data);
          }
        }

        Context ctx;
        std::vector<Layer *> childLayers;
        for (const WorkerData *data : workers) {
          data->dissector->analyze(&ctx, data->worker, layer);
          for (Layer *childLayer : layer->children()) {
            if (childLayer->confidence() >= d->confidenceThreshold) {
              /*
              if (result.streamIdentifier[0] != '\0') {
                streamedLayers.push_back(std::make_pair(
                    childLayer, std::string(result.streamIdentifier)));
              }
              */
              auto it = dissectedIds.find(childLayer->id());
              if (it == dissectedIds.end()) {
                nextlayers.push_back(childLayer);
              }
            }
          }
        }
        std::sort(childLayers.begin(), childLayers.end(),
                  [](const Layer *a, const Layer *b) {
                    return b->confidence() < a->confidence();
                  });
        for (Layer *child : childLayers) {
          layer->addLayer(child);
        }
      }
      leafLayers.swap(nextlayers);
    }
  }

  d->resolver->resolve(streamedLayers.data(), streamedLayers.size());

  if (d->callback) {
    d->callback(&frames.front(), size);
  }

  return true;
}

void DissectorThread::exit() { d->workers.clear(); }
}
