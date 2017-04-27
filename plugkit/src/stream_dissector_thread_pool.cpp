#include "stream_dissector_thread_pool.hpp"
#include "stream_dissector_thread.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "stream_logger.hpp"
#include "variant.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "private/chunk.hpp"
#include <thread>
#include <array>
#include <v8.h>
#include <uv.h>

namespace plugkit {

class StreamDissectorThreadPool::Private {
public:
  Private();
  ~Private();

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  std::vector<std::unique_ptr<StreamDissectorThread>> threads;
  std::vector<StreamDissectorFactoryConstPtr> dissectorFactories;
  FrameStorePtr store;
  Callback callback;
  std::thread thread;
  Variant options;
};

StreamDissectorThreadPool::Private::Private() {}

StreamDissectorThreadPool::Private::~Private() {}

StreamDissectorThreadPool::StreamDissectorThreadPool(const Variant &options,
                                                     const FrameStorePtr &store,
                                                     const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->store = store;
  d->callback = callback;
}

StreamDissectorThreadPool::~StreamDissectorThreadPool() {
  for (const auto &thread : d->threads) {
    thread->stop();
  }
  for (const auto &thread : d->threads) {
    thread->join();
  }
  d->store->close();
  if (d->thread.joinable())
    d->thread.join();
}

void StreamDissectorThreadPool::registerDissector(
    const StreamDissectorFactoryConstPtr &factory) {
  d->dissectorFactories.push_back(factory);
}

void StreamDissectorThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

void StreamDissectorThreadPool::start() {
  if (d->thread.joinable() || !d->threads.empty())
    return;

  int threads = std::thread::hardware_concurrency();
  for (int i = 0; i < threads; ++i) {
    auto dissectorThread = new StreamDissectorThread(d->options, d->callback);
    for (const auto &factory : d->dissectorFactories) {
      dissectorThread->pushStreamDissectorFactory(factory);
    }
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }

  d->thread = std::thread([this, threads]() {
    size_t offset = 0;
    std::array<FrameViewConstPtr, 128> views;
    while (true) {
      size_t size = d->store->dequeue(offset, views.size(), &views.front());
      if (size == 0)
        return;
      offset += size;

      std::function<std::vector<ChunkConstPtr>(const LayerConstPtr &)>
          findChunks = [&findChunks](
              const LayerConstPtr &layer) -> std::vector<ChunkConstPtr> {
        std::vector<ChunkConstPtr> chunks;
        const auto &list = layer->chunks();
        for (const auto &chunk : list) {
          chunk->d->setLayer(layer);
        }
        chunks.insert(chunks.begin(), list.begin(), list.end());
        for (const auto &child : layer->children()) {
          const auto &childList = findChunks(child);
          chunks.insert(chunks.begin(), childList.begin(), childList.end());
        }
        return chunks;
      };

      std::vector<std::vector<ChunkConstPtr>> chunkMap;
      chunkMap.resize(threads);
      for (size_t i = 0; i < size; ++i) {
        const auto &view = views[i];
        for (const auto &chunk : findChunks(view->frame()->rootLayer())) {
          int thread =
              std::hash<std::string>{}(chunk->streamId()) % d->threads.size();
          chunkMap[thread].push_back(chunk);
        }
      }
      for (size_t i = 0; i < chunkMap.size(); ++i) {
        const auto &chunks = chunkMap[i];
        d->threads[i]->push(&chunks.front(), chunks.size());
      }
    }
  });
}
}
