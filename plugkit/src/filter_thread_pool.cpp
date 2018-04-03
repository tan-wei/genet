#include "filter_thread_pool.hpp"
#include "filter_thread.hpp"
#include "random_id.hpp"
#include "session_context.hpp"
#include "variant.hpp"
#include <map>
#include <uv.h>

namespace plugkit {

class FilterThreadPool::Private {
public:
  Private(const SessionContext *sctx,
          const std::string &body,
          const FrameStorePtr &store,
          const Callback &callback);
  ~Private();

public:
  const SessionContext *sctx;
  std::vector<std::unique_ptr<FilterThread>> threads;
  std::map<uint32_t, bool> sequence;
  std::vector<uint32_t> frames;
  uint32_t maxSeq = 0;
  uv_rwlock_t rwlock;
  const std::string body;
  const FrameStorePtr store;
  const Callback callback;
  const std::string inspectorId = ":" + RandomID::generate<8>();
  InspectorCallback inspectorCallback;
  std::vector<std::string> inspectors;
};

FilterThreadPool::Private::Private(const SessionContext *sctx,
                                   const std::string &body,
                                   const FrameStorePtr &store,
                                   const Callback &callback)
    : sctx(sctx), body(body), store(store), callback(callback) {
  uv_rwlock_init(&rwlock);
}

FilterThreadPool::Private::~Private() { uv_rwlock_destroy(&rwlock); }

FilterThreadPool::FilterThreadPool(const SessionContext *sctx,
                                   const std::string &body,
                                   const FrameStorePtr &store,
                                   const Callback &callback)
    : d(new Private(sctx, body, store, callback)) {}

FilterThreadPool::~FilterThreadPool() {
  for (const auto &thread : d->threads) {
    thread->close();
  }
  for (const auto &thread : d->threads) {
    thread->join();
  }
}

void FilterThreadPool::sendInspectorMessage(const std::string &id,
                                            const std::string &msg) {
  for (size_t i = 0; i < d->inspectors.size(); ++i) {
    if (d->inspectors[i] == id) {
      d->threads[i]->sendInspectorMessage(msg);
      break;
    }
  }
}

std::vector<std::string> FilterThreadPool::inspectors() const {
  return d->inspectors;
}

void FilterThreadPool::start() {
  auto threadCallback = [this](uint32_t begin,
                               const std::vector<char> &results) {
    uv_rwlock_wrlock(&d->rwlock);
    for (const auto &match : results) {
      d->sequence.insert(std::make_pair(begin, static_cast<bool>(match)));
      begin++;
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

  int concurrency = std::stoi(d->sctx->config()["_.dissector.concurrency"]);
  if (concurrency == 0)
    concurrency = std::thread::hardware_concurrency();
  if (concurrency == 0)
    concurrency = 1;

  for (int i = 0; i < concurrency; ++i) {
    auto thread = new FilterThread(d->body, d->store, threadCallback);

    const auto &inspector =
        "worker:filter:" + std::to_string(i) + d->inspectorId;
    thread->setInspector(inspector, [this, inspector](const std::string &msg) {
      d->inspectorCallback(inspector, msg);
    });
    d->inspectors.push_back(inspector);

    d->threads.emplace_back(thread);
  }
  for (const auto &thread : d->threads) {
    thread->start();
  }
}

void FilterThreadPool::setInspectorCallback(const InspectorCallback &callback) {
  d->inspectorCallback = callback;
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
} // namespace plugkit
