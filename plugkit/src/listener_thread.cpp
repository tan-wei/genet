#include "listener_thread.hpp"
#include "listener_status.hpp"
#include "listener.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include <array>

namespace plugkit {

class ListenerThread::Private {
public:
  FrameStorePtr store;
  ListenerStatusPtr status;
  Callback callback;
  ListenerPtr listener;
  size_t offset = 0;
};

ListenerThread::ListenerThread(ListenerPtr &&listener,
                               const ListenerStatusPtr &status,
                               const FrameStorePtr &store,
                               const Callback &callback)
    : d(new Private()) {
  d->store = store;
  d->status = status;
  d->callback = callback;
  d->listener = (std::move(listener));
}

ListenerThread::~ListenerThread() {}

void ListenerThread::enter() {}

bool ListenerThread::loop() {
  std::thread::id id = std::this_thread::get_id();
  std::array<const FrameView *, 128> views;
  size_t size = d->store->dequeue(d->offset, views.size(), &views[0], id);
  if (size == 0)
    return false;

  bool updated = false;
  for (size_t i = 0; i < size; ++i) {
    const auto &view = views[i];
    if (d->listener->analyze(view)) {
      const std::vector<AttributeConstPtr> &attributes =
          d->listener->attributes();
      const std::vector<ChunkConstPtr> &chunks = d->listener->chunks();
      for (const auto &attr : attributes) {
        d->status->addAttribute(attr);
      }
      for (const auto &chunk : chunks) {
        d->status->addChunk(chunk);
      }
      if (!attributes.empty() || !chunks.empty()) {
        updated = true;
      }
    }
  }

  if (updated) {
    d->callback();
  }
  d->offset += size;
  return true;
}

void ListenerThread::exit() {}

void ListenerThread::close() {
  std::thread::id id = thread.get_id();
  if (id != std::thread::id()) {
    d->store->close(id);
  }
}
}
