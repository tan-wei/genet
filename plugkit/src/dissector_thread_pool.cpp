#include "dissector_thread_pool.hpp"
#include "dissector.h"
#include "dissector_thread.hpp"
#include "variant.hpp"
#include <array>

namespace plugkit {

class DissectorThreadPool::Private {
public:
  std::vector<std::unique_ptr<DissectorThread>> threads;
  std::vector<Dissector> dissectors;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  FrameQueuePtr queue = std::make_shared<FrameQueue>();
  VariantMap options;
  Callback callback;
  RootAllocator *allocator = nullptr;
};

DissectorThreadPool::DissectorThreadPool() : d(new Private()) {}

DissectorThreadPool::~DissectorThreadPool() {
  d->queue->close();
  for (const auto &thread : d->threads) {
    thread->join();
  }
}

void DissectorThreadPool::start() {
  if (!d->threads.empty())
    return;

  auto threadCallback = [this](Frame **begin, size_t size) {
    d->callback(begin, size);
  };

  int concurrency = d->options["_.dissector.concurrency"].uint32Value(0);
  if (concurrency == 0)
    concurrency = std::thread::hardware_concurrency();
  if (concurrency == 0)
    concurrency = 1;

  for (int i = 0; i < concurrency; ++i) {
    auto dissectorThread =
        new DissectorThread(d->options, d->queue, threadCallback);
    for (const auto &diss : d->dissectors) {
      dissectorThread->pushDissector(diss);
    }
    dissectorThread->setAllocator(d->allocator);
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
}

void DissectorThreadPool::setOptions(const VariantMap &options) {
  d->options = options;
}

void DissectorThreadPool::setCallback(const Callback &callback) {
  d->callback = callback;
}

void DissectorThreadPool::setAllocator(RootAllocator *allocator) {
  d->allocator = allocator;
}

void DissectorThreadPool::registerDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void DissectorThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

void DissectorThreadPool::push(Frame **begin, size_t length) {
  d->queue->enqueue(begin, begin + length);
}
} // namespace plugkit
