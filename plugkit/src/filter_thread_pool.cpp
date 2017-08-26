#include "filter_thread_pool.hpp"
#include "filter_thread.hpp"
#include "variant.hpp"
#include <map>
#include <uv.h>

namespace plugkit {

class FilterThreadPool::Private {
public:
  Private(const std::string &body, const Variant &options,
          const FrameStorePtr &store, const Callback &callback);
  ~Private();

public:
  std::vector<std::unique_ptr<FilterThread>> threads;
  std::map<uint32_t, bool> sequence;
  std::vector<uint32_t> frames;
  uint32_t maxSeq = 0;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  uv_rwlock_t rwlock;
  const std::string body;
  const Variant options;
  const FrameStorePtr store;
  const Callback callback;
};

FilterThreadPool::Private::Private(const std::string &body,
                                   const Variant &options,
                                   const FrameStorePtr &store,
                                   const Callback &callback)
    : body(body), options(options), store(store), callback(callback) {
  uv_rwlock_init(&rwlock);
}

FilterThreadPool::Private::~Private() { uv_rwlock_destroy(&rwlock); }

FilterThreadPool::FilterThreadPool(const std::string &body,
                                   const Variant &options,
                                   const FrameStorePtr &store,
                                   const Callback &callback)
    : d(new Private(body, options, store, callback)) {}

FilterThreadPool::~FilterThreadPool() {
  for (const auto &thread : d->threads) {
    thread->close();
  }
  for (const auto &thread : d->threads) {
    thread->join();
  }
}

void FilterThreadPool::start() {
  auto threadCallback = [this](
      const std::vector<std::pair<uint32_t, bool>> &results) {
    uv_rwlock_wrlock(&d->rwlock);
    for (const auto &pair : results) {
      d->sequence.insert(pair);
    }
    uint32_t maxSeq = d->maxSeq;
    auto end = d->sequence.begin();
    for (auto it = d->sequence.find(maxSeq + 1); it != d->sequence.end();
         end = it, it = d->sequence.find(++maxSeq + 1)) {
      if (it->second) {
        d->frames.push_back(it->first);
      }
    }
    if (d->maxSeq < maxSeq) {
      d->maxSeq = maxSeq;
      d->sequence.erase(d->sequence.begin(), end);
      d->callback();
    }
    uv_rwlock_wrunlock(&d->rwlock);
  };

  int concurrency = d->options["_"]["concurrency"].uint64Value(0);
  if (concurrency == 0)
    concurrency = std::thread::hardware_concurrency();
  if (concurrency == 0)
    concurrency = 1;

  for (int i = 0; i < concurrency; ++i) {
    auto thread = new FilterThread(d->body, d->store, threadCallback);
    thread->setLogger(d->logger);
    d->threads.emplace_back(thread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
}

void FilterThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

std::vector<uint32_t> FilterThreadPool::get(uint32_t offset,
                                            uint32_t length) const {
  std::vector<uint32_t> list;
  uv_rwlock_rdlock(&d->rwlock);
  for (size_t i = offset; i < offset + length && i < d->frames.size(); ++i) {
    list.push_back(d->frames[i]);
  }
  uv_rwlock_rdunlock(&d->rwlock);
  return list;
}

uint32_t FilterThreadPool::size() const {
  uv_rwlock_rdlock(&d->rwlock);
  uint32_t size = d->frames.size();
  uv_rwlock_rdunlock(&d->rwlock);
  return size;
}

uint32_t FilterThreadPool::maxSeq() const {
  uv_rwlock_rdlock(&d->rwlock);
  uint32_t maxSeq = d->maxSeq;
  uv_rwlock_rdunlock(&d->rwlock);
  return maxSeq;
}
}
