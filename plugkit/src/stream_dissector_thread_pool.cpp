#include "stream_dissector_thread_pool.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "private/chunk.hpp"
#include "stream_dissector_thread.hpp"
#include "stream_logger.hpp"
#include "variant.hpp"
#include <array>
#include <thread>
#include <uv.h>
#include <v8.h>

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
    std::array<const FrameView *, 128> views;
    while (true) {
      size_t size = d->store->dequeue(offset, views.size(), &views.front());
      if (size == 0)
        return;
      offset += size;

      std::function<std::vector<Layer *>(Layer *)> findStreamedLayers =
          [&findStreamedLayers](Layer *layer) -> std::vector<Layer *> {

        std::vector<Layer *> layers;
        if (layer->children().empty()) {
          if (!layer->streamId().empty()) {
            layers.push_back(layer);
          }
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
        const auto &view = views[i];
        const auto &layers = findStreamedLayers(view->frame()->rootLayer());
        for (Layer *layer : layers) {
          int thread =
              std::hash<std::string>{}(layer->streamId()) % d->threads.size();
          layerMap[thread].push_back(layer);
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

uint32_t StreamDissectorThreadPool::queue() const {
  uint32_t size = 0;
  for (const auto &thread : d->threads) {
    thread->queue();
  }
  return size;
}
}
