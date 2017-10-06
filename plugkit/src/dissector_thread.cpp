#include "dissector_thread.hpp"
#include "context.hpp"
#include "dissector.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "tag_filter.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace plugkit {

namespace {

struct WorkerData {
  const Dissector *dissector;
  TagFilter filter;
  Worker worker;
};
} // namespace

class DissectorThread::Private {
public:
  Private(const Variant &options, const FrameQueuePtr &queue,
          const Callback &callback);
  ~Private();

public:
  std::vector<Dissector> dissectors;
  std::vector<WorkerData> workers;
  double confidenceThreshold;

  Context ctx;
  const Variant options;
  const FrameQueuePtr queue;
  const Callback callback;
};

DissectorThread::Private::Private(const Variant &options,
                                  const FrameQueuePtr &queue,
                                  const Callback &callback)
    : options(options), queue(queue), callback(callback) {
  ctx.options = options;
}

DissectorThread::Private::~Private() {}

DissectorThread::DissectorThread(const Variant &options,
                                 const FrameQueuePtr &queue,
                                 const Callback &callback)
    : d(new Private(options, queue, callback)) {
  d->confidenceThreshold =
      options["_"]["confidenceThreshold"].uint64Value(0) / 100.0;
}

DissectorThread::~DissectorThread() {}

void DissectorThread::pushDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void DissectorThread::enter() {

  for (auto &diss : d->dissectors) {
    if (diss.initialize) {
      diss.initialize(&d->ctx, &diss);
    }
  }

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
      data.worker = diss.createWorker(&d->ctx, &diss);
    }
    d->workers.push_back(data);
  }
}

bool DissectorThread::loop() {
  std::array<Frame *, 128> frames;
  size_t size = d->queue->dequeue(std::begin(frames), frames.size());
  if (size == 0)
    return false;

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

        for (const WorkerData *data : workers) {
          data->dissector->analyze(&d->ctx, data->dissector, data->worker,
                                   layer);
          for (Layer *childLayer : layer->layers()) {
            if (childLayer->confidence() >= d->confidenceThreshold) {
              auto it = dissectedIds.find(childLayer->id());
              if (it == dissectedIds.end()) {
                nextlayers.push_back(childLayer);
              }
            }
          }
        }
      }
      leafLayers.swap(nextlayers);
    }
  }

  if (d->callback) {
    d->callback(&frames.front(), size);
  }

  return true;
}

void DissectorThread::exit() {
  for (auto &diss : d->dissectors) {
    if (diss.terminate) {
      diss.terminate(&d->ctx, &diss);
    }
  }
  d->workers.clear();
}
} // namespace plugkit
