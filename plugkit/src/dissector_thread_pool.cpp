#include "dissector_thread_pool.hpp"
#include "dissector.hpp"
#include "dissector_thread.hpp"
#include "stream_resolver.hpp"
#include "frame.hpp"
#include "variant.hpp"
#include <uv.h>
#include <v8.h>

namespace plugkit {

class DissectorThreadPool::Private {
public:
  Private();
  ~Private();

public:
  std::vector<std::unique_ptr<DissectorThread>> threads;
  std::vector<DissectorFactoryConstPtr> dissectorFactories;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  FrameQueuePtr queue = std::make_shared<FrameQueue>();
  StreamResolverPtr resolver = std::make_shared<StreamResolver>();
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

  auto threadCallback = [this](Frame **begin, size_t size) {
    d->callback(begin, size);
  };

  int threads = std::thread::hardware_concurrency();
  for (int i = 0; i < threads; ++i) {
    auto dissectorThread =
        new DissectorThread(d->options, d->queue, d->resolver, threadCallback);
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

void DissectorThreadPool::push(Frame **begin, size_t length) {
  d->queue->enqueue(begin, begin + length);
}
}
