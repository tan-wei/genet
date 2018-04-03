#include "stream_dissector_thread.hpp"
#include "context.hpp"
#include "dissector.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "queue.hpp"
#include "sandbox.hpp"
#include "session_context.hpp"
#include "variant.hpp"

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
  Private(const SessionContext *sctx, const Callback &callback);
  ~Private();
  void analyze(Layer *layer, std::vector<Layer *> *nextLayers);

public:
  Queue<Layer *> queue;
  std::vector<Dissector> dissectors;
  using IdMap = std::unordered_map<uint32_t, WorkerContext>;
  std::unordered_map<Token, IdMap> workers;

  Context ctx;
  const Callback callback;
};

StreamDissectorThread::Private::Private(const SessionContext *sctx,
                                        const Callback &callback)
    : ctx(sctx), callback(callback) {}
StreamDissectorThread::Private::~Private() {}

StreamDissectorThread::StreamDissectorThread(const SessionContext *sctx,
                                             const Callback &callback)
    : d(new Private(sctx, callback)) {
  d->ctx.confidenceThreshold = static_cast<LayerConfidence>(
      std::stoi(sctx->config()["_.dissector.confidenceThreshold"]));
}

StreamDissectorThread::~StreamDissectorThread() {}

void StreamDissectorThread::pushStreamDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void StreamDissectorThread::enter() {
  for (auto &diss : d->dissectors) {
    if (diss.initialize) {
      diss.initialize(&d->ctx, &diss);
    }
  }

  Sandbox::activate(Sandbox::PROFILE_DISSECTOR);
}

void StreamDissectorThread::Private::analyze(Layer *layer,
                                             std::vector<Layer *> *nextLayers) {
  std::unordered_set<Token> dissectedIds;

  Token id = layer->id();
  dissectedIds.insert(id);
  auto &streamWorkers = workers[id][layer->worker()];

  for (const Dissector &diss : dissectors) {
    auto it = streamWorkers.workers.find(&diss);
    if (it == streamWorkers.workers.end()) {
      Worker worker = {nullptr};
      if (diss.createWorker) {
        worker = diss.createWorker(&ctx, &diss);
      }
      streamWorkers.workers[&diss] = worker;
    }
  }

  std::vector<std::pair<const Dissector *, Worker>> closedWorkers;
  std::unordered_set<const Dissector *> usedDissectors;

  while (true) {
    bool dissectable = false;

    std::unordered_set<Token> tags;
    for (const Token &token : layer->tags()) {
      tags.insert(token);
    }

    for (const auto &pair : streamWorkers.workers) {
      const Dissector *diss = pair.first;
      Worker worker = pair.second;

      bool match = false;
      for (const Token &token : diss->layerHints) {
        if (token != Token_null()) {
          auto it = tags.find(token);
          if (it == tags.end()) {
            match = false;
            break;
          }
          match = true;
        }
      }
      if (!match) {
        continue;
      }

      if (usedDissectors.find(diss) != usedDissectors.end()) {
        continue;
      }
      usedDissectors.insert(diss);
      dissectable = true;

      if (diss->examine(&ctx, diss, worker, layer) < ctx.confidenceThreshold) {
        continue;
      }
      diss->analyze(&ctx, diss, worker, layer);
      if (ctx.closeStream) {
        closedWorkers.emplace_back(diss, worker);
        ctx.closeStream = false;
      }

      for (Layer *childLayer : layer->layers()) {
        auto it = dissectedIds.find(childLayer->id());
        if (it == dissectedIds.end()) {
          nextLayers->push_back(childLayer);
        }
      }
    }
    if (!dissectable) {
      break;
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

  while (!layers.empty()) {
    std::vector<Layer *> nextLayers;

    for (size_t i = 0; i < layers.size(); ++i) {
      d->analyze(layers[i], &nextLayers);
    }

    layers.swap(nextLayers);
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

void StreamDissectorThread::push(Layer **begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }
} // namespace plugkit
