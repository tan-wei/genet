#include "stream_dissector_thread_pool.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "stream_dissector_thread.hpp"
#include "stream_logger.hpp"
#include "variant.hpp"
#include "dissector.hpp"
#include "variant.hpp"
#include <array>
#include <thread>
#include <uv.h>

namespace plugkit {

class StreamDissectorThreadPool::Private {
public:
  Private();
  ~Private();
  uint32_t updateIndex(int thread, uint32_t pushed = 0, uint32_t dissected = 0);

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  std::vector<std::unique_ptr<StreamDissectorThread>> threads;
  std::vector<Dissector> dissectors;
  FrameStorePtr store;
  Callback callback;
  std::thread thread;
  Variant options;

  std::vector<std::pair<uint32_t, uint32_t>> threadStats;
  std::mutex mutex;
};

StreamDissectorThreadPool::Private::Private() {}

StreamDissectorThreadPool::Private::~Private() {}

uint32_t StreamDissectorThreadPool::Private::updateIndex(int thread,
                                                         uint32_t pushed,
                                                         uint32_t dissected) {
  std::lock_guard<std::mutex> lock(mutex);

  uint32_t min = 0;
  if (threadStats[thread].first < pushed) {
    threadStats[thread].first = pushed;
  }
  if (threadStats[thread].second < dissected) {
    threadStats[thread].second = dissected;
  }

  for (const auto &pair : threadStats) {
    if (min < pair.first) {
      min = pair.first;
    }
  }

  for (const auto &pair : threadStats) {
    if (pair.first > pair.second) {
      if (min > pair.second) {
        min = pair.second;
      }
    }
  }

  return min;
}

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

void StreamDissectorThreadPool::registerDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void StreamDissectorThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

void StreamDissectorThreadPool::start() {
  if (d->thread.joinable() || !d->threads.empty())
    return;

  int threads = std::thread::hardware_concurrency();
  for (int i = 0; i < threads; ++i) {
    auto dissectorThread = new StreamDissectorThread(
        d->options, [this, i](uint32_t maxFrameIndex) {
          uint32_t index = d->updateIndex(i, 0, maxFrameIndex);
          d->callback(index);
        });
    for (const auto &diss : d->dissectors) {
      dissectorThread->pushStreamDissector(diss);
    }
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }

  d->threadStats.resize(d->threads.size());

  for (const auto &thread : d->threads) {
    thread->start();
  }

  d->thread = std::thread([this, threads]() {
    size_t offset = 0;
    std::array<const Frame *, 128> frames;
    while (true) {
      size_t size = d->store->dequeue(offset, frames.size(), &frames.front());
      if (size == 0)
        return;
      offset += size;

      std::function<std::vector<Layer *>(Layer *)> findStreamedLayers =
          [&findStreamedLayers](Layer *layer) -> std::vector<Layer *> {

        std::vector<Layer *> layers;
        if (layer->children().empty()) {
          layers.push_back(layer);
        } else {
          for (const auto &child : layer->children()) {
            const auto &childList = findStreamedLayers(child);
            layers.insert(layers.begin(), childList.begin(), childList.end());
          }
        }
        return layers;
      };

      std::vector<std::vector<Layer *>> layerMap;
      layerMap.resize(threads);
      for (size_t i = 0; i < size; ++i) {
        const auto &layers = findStreamedLayers(frames[i]->rootLayer());
        for (Layer *layer : layers) {
          int thread = layer->streamId() % d->threads.size();
          layerMap[thread].push_back(layer);
          if (const Frame *frame = layer->frame()) {
            d->updateIndex(thread, frame->index(), 0);
          }
        }
      }
      for (size_t i = 0; i < layerMap.size(); ++i) {
        auto &layer = layerMap[i];
        if (!layer.empty()) {
          d->threads[i]->push(&layer.front(), layer.size());
        }
      }
    }
  });
}
}
