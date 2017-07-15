#include "dissector_thread.hpp"
#include "dissector.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "session_context.hpp"
#include "variant.hpp"
#include <array>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <v8.h>

namespace plugkit {

class DissectorThread::Private {
public:
  FrameQueuePtr queue;
  Callback callback;
  std::vector<DissectorFactoryConstPtr> factories;
  std::unordered_map<Dissector::WorkerPtr, std::vector<minins>> workers;
  Variant options;
  std::atomic<uint32_t> queuedFrames;
  double confidenceThreshold;
};

DissectorThread::DissectorThread(const Variant &options,
                                 const FrameQueuePtr &queue,
                                 const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->queue = queue;
  d->callback = callback;
  d->confidenceThreshold =
      options["_"]["confidenceThreshold"].uint64Value(0) / 100.0;
}

DissectorThread::~DissectorThread() {}

void DissectorThread::pushDissectorFactory(
    const DissectorFactoryConstPtr &factory) {
  d->factories.push_back(factory);
}

void DissectorThread::enter() {
  SessionContext ctx;
  ctx.logger = logger;
  ctx.options = d->options;
  for (const auto &factory : d->factories) {
    auto diss = factory->create(ctx);
    if (auto worker = diss->createWorker()) {
      d->workers[std::move(worker)] = diss->namespaces();
    }
  }
}

bool DissectorThread::loop() {
  std::array<Frame *, 128> frames;
  d->queuedFrames.store(0, std::memory_order_relaxed);
  size_t size = d->queue->dequeue(std::begin(frames), frames.size());
  d->queuedFrames.store(size, std::memory_order_relaxed);
  if (size == 0)
    return false;

  for (size_t i = 0; i < size; ++i) {
    std::unordered_set<minins> dissectedNamespaces;

    Frame *frame = frames[i];
    const auto &rootLayer = frame->rootLayer();
    if (!rootLayer)
      continue;

    std::vector<Dissector::Worker *> workers;
    std::vector<Layer *> leafLayers = {rootLayer};
    while (!leafLayers.empty()) {
      std::vector<Layer *> nextlayers;
      for (const auto &layer : leafLayers) {
        const minins &ns = layer->ns();
        dissectedNamespaces.insert(ns);

        workers.clear();
        for (const auto &pair : d->workers) {
          for (const auto &filter : pair.second) {
            if (ns.match(filter)) {
              workers.push_back(pair.first.get());
            }
          }
        }

        std::vector<Layer *> childLayers;
        for (auto *worker : workers) {
          if (Layer *childLayer = worker->analyze(layer)) {
            if (childLayer->confidence() >= d->confidenceThreshold) {
              childLayer->setParent(layer);
              childLayer->setFrame(frame);
              childLayers.push_back(childLayer);
              if (dissectedNamespaces.count(childLayer->ns()) == 0) {
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
          layer->addChild(child);
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

void DissectorThread::exit() { d->workers.clear(); }

uint32_t DissectorThread::queue() const {
  return d->queuedFrames.load(std::memory_order_relaxed);
}
}
