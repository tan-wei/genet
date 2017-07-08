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
  Queue<const Chunk *> queue;
  std::vector<StreamDissectorFactoryConstPtr> factories;
  std::unordered_map<StreamDissectorPtr, std::vector<strns>> dissectors;
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
  std::vector<const Chunk *> chunks;
  chunks.resize(128);
  d->queuedFrames.store(0, std::memory_order_relaxed);
  size_t size = d->queue.dequeue(std::begin(chunks), chunks.capacity());
  d->queuedFrames.store(size, std::memory_order_relaxed);
  if (size == 0)
    return false;

  chunks.resize(size);

  for (size_t i = 0; i < chunks.size(); ++i) {
    const auto chunk = chunks[i];
    const auto &subChunks = processChunk(chunk);
    for (const auto &sub : subChunks) {
      sub->d->setLayer(chunk->layer());
    }
    chunks.insert(chunks.end(), subChunks.begin(), subChunks.end());
    d->queuedFrames.store(chunks.size(), std::memory_order_relaxed);
    d->count++;
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

void StreamDissectorThread::push(const Chunk *const *begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }

std::vector<const Chunk *>
StreamDissectorThread::processChunk(const Chunk *chunk) {
  const strns &ns = strns(chunk->streamNs());
  auto &workers = d->workers[ns][chunk->streamId()];

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

  std::vector<const Chunk *> subChunks;

  std::function<std::vector<const Chunk *>(const Layer *)> findChunks =
      [&findChunks](const Layer *layer) -> std::vector<const Chunk *> {
    std::vector<const Chunk *> chunks;
    const auto &list = layer->chunks();
    chunks.insert(chunks.begin(), list.begin(), list.end());
    for (const auto &child : layer->children()) {
      const auto &childList = findChunks(child);
      chunks.insert(chunks.begin(), childList.begin(), childList.end());
    }
    return chunks;
  };

  std::vector<Frame *> frames;
  for (const auto &worker : workers.list) {
    if (const auto &layer = worker->analyze(chunk)) {
      if (!layer->ns().empty()) {
        auto frame = Frame::Private::create();
        frame->d->setLength(layer->payload().size());
        frame->d->setRootLayer(layer);
        frames.push_back(std::move(frame));
      }
      subChunks = findChunks(layer);
    }
  }

  if (!workers.list.empty()) {
    workers.lastUpdated = std::chrono::system_clock::now();
  }

  d->callback(&frames.front(), frames.size());
  return subChunks;
}

uint32_t StreamDissectorThread::queue() const {
  return d->queuedFrames.load(std::memory_order_relaxed);
}
}
