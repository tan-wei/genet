#include "filter_thread.hpp"
#include "filter.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "null_logger.hpp"
#include <array>
#include <uv.h>
#include <v8.h>

namespace plugkit {

class FilterThread::Private {
public:
  FrameStorePtr store;
  Callback callback;
  std::string body;
  std::unique_ptr<Filter> filter;
  size_t offset = 0;
};

FilterThread::FilterThread(const std::string &body, const FrameStorePtr &store,
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

  std::vector<std::pair<uint32_t, bool>> results;
  for (size_t i = 0; i < size; ++i) {
    const auto &view = views[i];
    bool match = d->filter->test(view);
    results.push_back(std::make_pair(view->frame()->index(), match));
  }

  d->callback(results);
  d->offset += size;
  return true;
}

void FilterThread::close() {
  std::thread::id id = thread.get_id();
  if (id != std::thread::id()) {
    d->store->close(id);
  }
}
}
