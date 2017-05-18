#include "dissector_thread_pool.hpp"
#include "dissector_thread.hpp"
#include "dissector.hpp"
#include "variant.hpp"
#include "frame.hpp"
#include <v8.h>
#include <uv.h>

namespace plugkit {

class DissectorThreadPool::Private {
public:
  Private();
  ~Private();

public:
  std::vector<std::unique_ptr<DissectorThread>> threads;
  std::vector<DissectorFactoryConstPtr> dissectorFactories;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  FrameUniqueQueuePtr queue = std::make_shared<FrameUniqueQueue>();
  Callback callback;
  Variant options;
};

DissectorThreadPool::Private::Private() {}

DissectorThreadPool::Private::~Private() {}

DissectorThreadPool::DissectorThreadPool(const Variant &options,
                                         const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->callback = callback;
}

DissectorThreadPool::~DissectorThreadPool() {
  d->queue->close();
  for (const auto &thread : d->threads) {
    thread->join();
  }
}

void DissectorThreadPool::start() {
  if (!d->threads.empty())
    return;

  auto threadCallback = [this](FrameUniquePtr *begin, size_t size) {
    d->callback(begin, size);
  };

  int threads = std::thread::hardware_concurrency();
  for (int i = 0; i < threads; ++i) {
    auto dissectorThread =
        new DissectorThread(d->options, d->queue, threadCallback);
    for (const auto &factory : d->dissectorFactories) {
      dissectorThread->pushDissectorFactory(factory);
    }
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
}

void DissectorThreadPool::registerDissector(
    const DissectorFactoryConstPtr &factory) {
  d->dissectorFactories.push_back(factory);
}

void DissectorThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

void DissectorThreadPool::push(FrameUniquePtr *begin, size_t length) {
  d->queue->enqueue(begin, begin + length);
}

uint32_t DissectorThreadPool::queue() const {
  uint32_t size = d->queue->size();
  for (const auto &thread : d->threads) {
    thread->queue();
  }
  return size;
}
}
