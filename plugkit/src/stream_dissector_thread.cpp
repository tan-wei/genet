#include "stream_dissector_thread.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "queue.hpp"
#include "variant.hpp"

#include "context.hpp"
#include "dissector.hpp"

#include <algorithm>
#include <cstring>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <v8.h>

namespace plugkit {

namespace {
struct WorkerContext {
  std::vector<const Dissector *> dissectors;
  std::map<const Dissector *, Worker> workers;
};
} // namespace

class StreamDissectorThread::Private {
public:
  Private(const VariantMap &options, const Callback &callback);
  ~Private();
  void analyze(Layer *layer,
               bool subLayer,
               std::vector<Layer *> *nextLayers,
               std::vector<Layer *> *nextSubLayers);

public:
  Queue<Layer *> queue;
  std::vector<Dissector> dissectors;
  LayerConfidence confidenceThreshold;
  using IdMap = std::unordered_map<uint32_t, WorkerContext>;
  std::unordered_map<Token, IdMap> workers;

  Context ctx;
  const VariantMap options;
  const Callback callback;
};

StreamDissectorThread::Private::Private(const VariantMap &options,
                                        const Callback &callback)
    : options(options), callback(callback) {
  ctx.options = options;
}
StreamDissectorThread::Private::~Private() {}

StreamDissectorThread::StreamDissectorThread(const VariantMap &options,
                                             const Callback &callback)
    : d(new Private(options, callback)) {
  d->confidenceThreshold = static_cast<LayerConfidence>(
      options["_.dissector.confidenceThreshold"].uint64Value(2));
}

StreamDissectorThread::~StreamDissectorThread() {}

void StreamDissectorThread::pushStreamDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void StreamDissectorThread::enter() {
  d->ctx.logger = logger;

  for (auto &diss : d->dissectors) {
    if (diss.initialize) {
      diss.initialize(&d->ctx, &diss);
    }
  }
}

void StreamDissectorThread::Private::analyze(
    Layer *layer,
    bool subLayer,
    std::vector<Layer *> *nextLayers,
    std::vector<Layer *> *nextSubLayers) {
  std::unordered_set<Token> dissectedIds;

  Token id = layer->id();
  dissectedIds.insert(id);
  auto &streamWorkers = workers[id][layer->worker()];

  if (streamWorkers.dissectors.empty()) {
    std::unordered_set<Token> tags;
    for (const Token &token : layer->tags()) {
      tags.insert(token);
    }

    for (const auto &diss : dissectors) {
      bool match = false;
      for (const Token &token : diss.layerHints) {
        if (token != Token_null()) {
          auto it = tags.find(token);
          if (it == tags.end()) {
            match = false;
            break;
          }
          match = true;
        }
      }
      if (match) {
        streamWorkers.dissectors.push_back(&diss);
      }
    }
  }

  for (const Dissector *diss : streamWorkers.dissectors) {
    auto it = streamWorkers.workers.find(diss);
    if (it == streamWorkers.workers.end()) {
      Worker worker = {nullptr};
      if (diss->createWorker) {
        worker = diss->createWorker(&ctx, diss);
      }
      streamWorkers.workers[diss] = worker;
    }
  }

  std::vector<std::pair<const Dissector *, Worker>> closedWorkers;
  if (subLayer) {
    for (const auto &pair : streamWorkers.workers) {
      const Dissector *diss = pair.first;
      Worker worker = pair.second;
      if (Layer *parent = layer->parent()) {
        diss->analyze(&ctx, diss, worker, parent);
      }
      if (ctx.closeStream) {
        closedWorkers.emplace_back(diss, worker);
        ctx.closeStream = false;
      }
    }
  } else {
    for (const auto &pair : streamWorkers.workers) {
      const Dissector *diss = pair.first;
      Worker worker = pair.second;
      diss->analyze(&ctx, diss, worker, layer);
      layer->removeUnconfidentLayers(&ctx, confidenceThreshold);
      if (ctx.closeStream) {
        closedWorkers.emplace_back(diss, worker);
        ctx.closeStream = false;
      }

      for (Layer *childLayer : layer->layers()) {
        if (childLayer->confidence() >= confidenceThreshold) {
          auto it = dissectedIds.find(childLayer->id());
          if (it == dissectedIds.end()) {
            nextLayers->push_back(childLayer);
          }
        }
      }
    }

    for (Layer *subLayer : layer->subLayers()) {
      if (subLayer->confidence() >= confidenceThreshold) {
        subLayer->setWorker(layer->worker());
        auto it = dissectedIds.find(subLayer->id());
        if (it == dissectedIds.end()) {
          nextSubLayers->push_back(subLayer);
        }
      }
    }
  }

  for (const auto &pair : closedWorkers) {
    const Dissector *diss = pair.first;
    Worker worker = pair.second;
    if (diss->destroyWorker) {
      diss->destroyWorker(&ctx, diss, worker);
    }
    streamWorkers.workers.erase(diss);
  }
}

bool StreamDissectorThread::loop() {
  std::vector<Layer *> layers;
  layers.resize(2048);

  size_t size = layers.size();
  const int waitFor = inspectorActivated() ? 0 : 3000;
  if (!d->queue.dequeue(std::begin(layers), &size, waitFor)) {
    return false;
  }

  if (size == 0) {
    return true;
  }

  layers.resize(size);

  uint32_t maxFrameIndex = 0;
  for (const Layer *layer : layers) {
    if (const Frame *frame = layer->frame()) {
      maxFrameIndex = std::max(maxFrameIndex, frame->index());
    }
  }

  std::vector<Layer *> subLayers;
  for (const Layer *layer : layers) {
    subLayers.insert(subLayers.end(), layer->subLayers().begin(),
                     layer->subLayers().end());
  }

  while (!layers.empty() || !subLayers.empty()) {
    std::vector<Layer *> nextLayers;
    std::vector<Layer *> nextSubLayers;

    for (size_t i = 0; i < subLayers.size(); ++i) {
      d->analyze(subLayers[i], true, &nextLayers, &nextSubLayers);
    }

    for (size_t i = 0; i < layers.size(); ++i) {
      d->analyze(layers[i], false, &nextLayers, &nextSubLayers);
    }

    layers.swap(nextLayers);
    subLayers.swap(nextSubLayers);
  }

  d->callback(maxFrameIndex);
  return true;
}

void StreamDissectorThread::exit() {
  for (auto &diss : d->dissectors) {
    if (diss.terminate) {
      diss.terminate(&d->ctx, &diss);
    }
  }
  d->dissectors.clear();
  d->workers.clear();
}

void StreamDissectorThread::setAllocator(RootAllocator *allocator) {
  d->ctx.rootAllocator = allocator;
}

void StreamDissectorThread::push(Layer **begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }
} // namespace plugkit
