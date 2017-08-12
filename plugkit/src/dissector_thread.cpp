#include "dissector_thread.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "stream_resolver.hpp"
#include "tag_filter.hpp"
#include "dissector.hpp"
#include "context.hpp"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstring>

namespace plugkit {

namespace {

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

    std::vector<Token> tags;
    for (Token tag : data.dissector->layerHints) {
      if (tag != Token_null()) {
        tags.push_back(tag);
      }
    }
    data.filter = TagFilter(tags);
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

        workers.clear();
        for (const auto &data : d->workers) {
          if (data.filter.match(layer->tags())) {
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
