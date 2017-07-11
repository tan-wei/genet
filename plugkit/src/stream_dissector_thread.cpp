#include "stream_dissector_thread.hpp"
#include "layer.hpp"
#include "private/chunk.hpp"
#include "private/frame.hpp"
#include "queue.hpp"
#include "session_context.hpp"
#include "stream_dissector.hpp"
#include "variant.hpp"
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <v8.h>

namespace plugkit {

class StreamDissectorThread::Private {
public:
  void cleanup();

public:
  Variant options;
  Callback callback;
  Queue<Layer *> queue;
  std::vector<StreamDissectorFactoryConstPtr> factories;
  std::unordered_map<StreamDissectorPtr, std::vector<strns>> dissectors;
  double confidenceThreshold;
  std::atomic<uint32_t> queuedFrames;
  size_t count = 0;

  struct WorkerList {
    std::vector<StreamDissector::WorkerPtr> list;
    StreamDissector::Worker::Timestamp lastUpdated;
  };

  using IdMap = std::unordered_map<std::string, WorkerList>;
  std::unordered_map<strns, IdMap> workers;
};

void StreamDissectorThread::Private::cleanup() {
  for (auto &pair : workers) {
    for (auto it = pair.second.begin(); it != pair.second.end();) {
      bool alive = false;
      for (auto &worker : it->second.list) {
        if (!worker->expired(it->second.lastUpdated)) {
          alive = true;
        }
      }
      if (alive) {
        ++it;
      } else {
        it = pair.second.erase(it);
      }
    }
  }
}

StreamDissectorThread::StreamDissectorThread(const Variant &options,
                                             const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->callback = callback;
  d->confidenceThreshold =
      options["_"]["confidenceThreshold"].uint64Value(0) / 100.0;
}

StreamDissectorThread::~StreamDissectorThread() {}

void StreamDissectorThread::pushStreamDissectorFactory(
    const StreamDissectorFactoryConstPtr &factory) {
  d->factories.push_back(factory);
}

void StreamDissectorThread::enter() {
  SessionContext ctx;
  ctx.logger = logger;
  ctx.options = d->options;
  for (const auto &factory : d->factories) {
    StreamDissectorPtr diss = factory->create(ctx);
    const auto &namespaces = diss->namespaces();
    d->dissectors[std::move(diss)] = namespaces;
  }
}

bool StreamDissectorThread::loop() {
  std::vector<Layer *> layers;
  layers.resize(128);
  d->queuedFrames.store(0, std::memory_order_relaxed);
  size_t size = d->queue.dequeue(std::begin(layers), layers.capacity());
  d->queuedFrames.store(size, std::memory_order_relaxed);
  if (size == 0)
    return false;

  layers.resize(size);

  while (!layers.empty()) {
    std::vector<Layer *> nextlayers;

    for (size_t i = 0; i < layers.size(); ++i) {
      std::unordered_set<strns> dissectedNamespaces;

      const auto layer = layers[i];
      const strns &ns = layer->ns();
      dissectedNamespaces.insert(ns);
      auto &workers = d->workers[ns][layer->streamId()];

      if (workers.list.empty()) {
        std::vector<StreamDissector *> dissectors;

        for (const auto &pair : d->dissectors) {
          for (const auto &filter : pair.second) {
            if (ns.match(filter)) {
              dissectors.push_back(pair.first.get());
            }
          }
        }

        for (auto dissector : dissectors) {
          if (auto worker = dissector->createWorker()) {
            workers.list.push_back(std::move(worker));
          }
        }
      }

      std::vector<Layer *> childLayers;
      for (const auto &worker : workers.list) {
        printf("%s\n", layer->streamId().c_str());
        if (Layer *childLayer = worker->analyze(layer)) {
          if (childLayer->confidence() >= d->confidenceThreshold) {
            childLayer->setParent(layer);

            childLayers.push_back(childLayer);
            if (!childLayer->streamId().empty()) {
              if (dissectedNamespaces.count(childLayer->ns()) == 0) {
                nextlayers.push_back(childLayer);
              }
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

      /*
      const auto &sublayers = processChunk(chunk);
      for (const auto &sub : sublayers) {
        sub->d->setLayer(chunk->layer());
      }
      layers.insert(layers.end(), sublayers.begin(), sublayers.end());
      d->queuedFrames.store(layers.size(), std::memory_order_relaxed);
      */
      d->count++;
    }

    layers.swap(nextlayers);
  }

  if (d->count % 1024 == 0) {
    d->cleanup();
  }
  return true;
}

void StreamDissectorThread::exit() {
  d->factories.clear();
  d->dissectors.clear();
  d->workers.clear();
}

void StreamDissectorThread::push(Layer **begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }

uint32_t StreamDissectorThread::queue() const {
  return d->queuedFrames.load(std::memory_order_relaxed);
}
}
