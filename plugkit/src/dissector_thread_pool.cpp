#include "dissector_thread_pool.hpp"
#include "dissector.hpp"
#include "dissector_thread.hpp"
#include "variant.hpp"
#include <array>

namespace plugkit {

class DissectorThreadPool::Private {
public:
  Private(const Variant &options, const Callback &callback);
  ~Private();

public:
  std::vector<std::unique_ptr<DissectorThread>> threads;
  std::vector<Dissector> dissectors;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  FrameQueuePtr queue = std::make_shared<FrameQueue>();
  const Variant options;
  const Callback callback;
};

DissectorThreadPool::Private::Private(const Variant &options,
                                      const Callback &callback)
    : options(options), callback(callback) {}

DissectorThreadPool::Private::~Private() {}

DissectorThreadPool::DissectorThreadPool(const Variant &options,
                                         const Callback &callback)
    : d(new Private(options, callback)) {}

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

  int concurrency = d->options["_"]["concurrency"].uint64Value(0);
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
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
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
