#include "stream_dissector_thread.hpp"
#include "layer.hpp"
#include "frame.hpp"
#include "queue.hpp"
#include "variant.hpp"

#include "dissector.hpp"
#include "context.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <v8.h>

namespace plugkit {

namespace {
struct WorkerContext {
  std::vector<std::pair<const Dissector *, void *>> list;
  std::chrono::system_clock::time_point lastUpdated =
      std::chrono::system_clock::now();
};
}

class StreamDissectorThread::Private {
public:
  void cleanup();
  void analyze(Layer *layer, bool subLayer, std::vector<Layer *> *nextLayers,
               std::vector<Layer *> *nextSubLayers);

public:
  Variant options;
  Callback callback;
  Queue<Layer *> queue;
  std::vector<Dissector> dissectors;
  double confidenceThreshold;
  size_t count = 0;

  using IdMap = std::unordered_map<uint32_t, WorkerContext>;
  std::unordered_map<Token, IdMap> workers;
};

void StreamDissectorThread::Private::cleanup() {
  Context ctx;
  ctx.options = options;
  for (auto &pair : workers) {
    for (auto it = pair.second.begin(); it != pair.second.end();) {
      bool alive = false;
      uint32_t elapsed =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now() - it->second.lastUpdated)
              .count();
      for (const auto &pair : it->second.list) {
        auto expired = pair.first->expired;
        if (expired) {
          if (!expired(&ctx, pair.second, elapsed)) {
            alive = true;
            break;
          }
        } else if (elapsed < 30000) {
          alive = true;
          break;
        }
      }
      if (alive) {
        ++it;
      } else {
        for (const auto &pair : it->second.list) {
          auto destroy = pair.first->destroyWorker;
          if (destroy) {
            destroy(&ctx, pair.second);
          }
        }
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

void StreamDissectorThread::pushStreamDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void StreamDissectorThread::enter() {}

void StreamDissectorThread::Private::analyze(
    Layer *layer, bool subLayer, std::vector<Layer *> *nextLayers,
    std::vector<Layer *> *nextSubLayers) {
  std::unordered_set<Token> dissectedIds;

  Token id = layer->id();
  dissectedIds.insert(id);
  auto &streamWorkers = workers[id][layer->streamId()];

  Context ctx;
  ctx.options = options;

  if (streamWorkers.list.empty()) {
    std::vector<const Dissector *> workerDissectors;

    std::unordered_set<Token> tags;
    for (const Token &token : layer->tags()) {
      tags.insert(token);
    }

    for (const auto &diss : dissectors) {
      bool match = true;
      for (const Token &token : diss.layerHints) {
        if (token != Token_null()) {
          auto it = tags.find(token);
          if (it == tags.end()) {
            match = false;
            break;
          }
        }
      }
      if (match) {
        workerDissectors.push_back(&diss);
      }

      for (const Dissector *diss : workerDissectors) {
        void *worker = nullptr;
        if (diss->createWorker) {
          worker = diss->createWorker(&ctx);
        }
        streamWorkers.list.push_back(std::make_pair(diss, worker));
      }
    }
  }

  if (subLayer) {
    for (const auto &pair : streamWorkers.list) {
      if (Layer *parent = layer->parent()) {
        pair.first->analyze(&ctx, pair.second, parent);
      }
    }
  } else {
    for (const auto &pair : streamWorkers.list) {
      pair.first->analyze(&ctx, pair.second, layer);

      for (Layer *childLayer : layer->children()) {
        if (childLayer->confidence() >= confidenceThreshold) {
          if (childLayer->streamId() > 0) {
            auto it = dissectedIds.find(childLayer->id());
            if (it == dissectedIds.end()) {
              nextLayers->push_back(childLayer);
            }
          }
        }
      }
    }
  }
}

bool StreamDissectorThread::loop() {
  std::vector<Layer *> layers;
  layers.resize(128);
  size_t size = d->queue.dequeue(std::begin(layers), layers.capacity());
  if (size == 0)
    return false;

  layers.resize(size);

  uint32_t maxFrameIndex = 0;
  for (const Layer *layer : layers) {
    if (const Frame *frame = layer->frame()) {
      maxFrameIndex = std::max(maxFrameIndex, frame->index());
    }
  }

  std::vector<Layer *> subLayers;
  while (!layers.empty() && !subLayers.empty()) {
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

  d->count++;
  if (d->count % 1024 == 0) {
    d->cleanup();
  }
  return true;
}

void StreamDissectorThread::exit() {
  d->dissectors.clear();
  d->workers.clear();
}

void StreamDissectorThread::push(Layer **begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }
}
