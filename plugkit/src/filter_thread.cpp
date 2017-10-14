#include "filter_thread.hpp"
#include "filter.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "null_logger.hpp"
#include <array>

namespace plugkit {

class FilterThread::Private {
public:
  FrameStorePtr store;
  Callback callback;
  std::string body;
  std::unique_ptr<Filter> filter;
  size_t offset = 0;
};

FilterThread::FilterThread(const std::string &body,
                           const FrameStorePtr &store,
                           const Callback &callback)
    : d(new Private()) {
  d->store = store;
  d->callback = callback;
  d->body = body;
}

FilterThread::~FilterThread() {}

void FilterThread::enter() { d->filter.reset(new Filter(d->body)); }

void FilterThread::exit() { d->filter.reset(); }

bool FilterThread::loop() {
  std::thread::id id = std::this_thread::get_id();
  std::array<const FrameView *, 128> views;
  size_t size = d->store->dequeue(d->offset, views.size(), &views[0], id);
  if (size == 0)
    return false;

  uint32_t begin = views[0]->frame()->index();
  std::vector<char> results;
  results.resize(size);
  d->filter->test(&results[0], &views[0], size);

  d->callback(begin, results);
  d->offset += size;
  return true;
}

void FilterThread::close() {
  std::thread::id id = thread.get_id();
  if (id != std::thread::id()) {
    d->store->close(id);
  }
}
} // namespace plugkit
