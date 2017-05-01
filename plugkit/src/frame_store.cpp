#include "frame_store.hpp"
#include "frame.hpp"
#include "frame_view.hpp"
#include <uv.h>
#include <atomic>
#include <map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

namespace plugkit {

class FrameStore::Private {
public:
  Private();
  ~Private();

public:
  std::atomic<size_t> size;
  std::map<uint32_t, FrameUniquePtr> sequence;
  std::vector<FrameViewConstPtr> frames;
  uint32_t maxSeq = 0;
  std::mutex mutex;
  std::condition_variable cond;
  std::unordered_set<std::thread::id> closedThreads;
  bool closed = false;
  Callback callback;
};

FrameStore::Private::Private() {}

FrameStore::Private::~Private() {}

FrameStore::FrameStore(const Callback &callback) : d(new Private()) {
  std::atomic_init(&d->size, 0lu);
  d->callback = callback;
}

FrameStore::~FrameStore() { close(); }

void FrameStore::insert(FrameUniquePtr *begin, size_t size) {
  std::lock_guard<std::mutex> lock(d->mutex);
  for (size_t i = 0; i < size; ++i) {
    d->sequence[begin[i]->index()] = std::move(begin[i]);
  }
  uint32_t maxSeq = d->maxSeq;
  auto end = d->sequence.begin();
  for (auto it = d->sequence.find(maxSeq + 1); it != d->sequence.end();
       end = it, it = d->sequence.find(++maxSeq + 1)) {
    d->frames.push_back(std::make_shared<FrameView>(std::move(it->second)));
  }
  if (d->maxSeq < maxSeq) {
    std::atomic_store_explicit(&d->size, d->frames.size(),
                               std::memory_order_relaxed);
    d->callback();
    d->maxSeq = maxSeq;
    d->sequence.erase(d->sequence.begin(), end);
    d->cond.notify_all();
  }
}

size_t FrameStore::dequeue(size_t offset, size_t max, FrameViewConstPtr *dst,
                           std::thread::id id) const {
  std::unique_lock<std::mutex> lock(d->mutex);
  size_t read = 0;
  uint32_t size = d->frames.size();
  if (size <= offset) {
    d->cond.wait(lock, [this, offset, id, &size]() -> bool {
      bool closed =
          ((id != std::thread::id()) && d->closedThreads.count(id) > 0);
      return d->closed || closed || ((size = d->frames.size()) > offset);
    });
  }
  if (id != std::thread::id() && d->closedThreads.count(id) > 0) {
    d->closedThreads.erase(id);
    return 0;
  }
  if (d->closed)
    return 0;
  for (size_t i = 0; i + offset < size && i < max; ++i, ++read) {
    dst[i] = d->frames[i + offset];
  }
  return read;
}

std::vector<FrameViewConstPtr> FrameStore::get(uint32_t offset,
                                               uint32_t length) const {
  std::vector<FrameViewConstPtr> frames;
  std::unique_lock<std::mutex> lock(d->mutex);
  for (size_t i = offset; i < offset + length && i < d->frames.size(); ++i) {
    frames.push_back(d->frames[i]);
  }
  return frames;
}

size_t FrameStore::size() const {
  return std::atomic_load_explicit(&d->size, std::memory_order_relaxed);
}

void FrameStore::close(std::thread::id id) {
  std::lock_guard<std::mutex> lock(d->mutex);
  if (id == std::thread::id()) {
    d->closed = true;
  } else {
    d->closedThreads.insert(id);
  }
  d->cond.notify_all();
}
}
