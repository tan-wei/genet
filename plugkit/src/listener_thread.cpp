#include "listener_thread.hpp"
#include "listener.hpp"

namespace plugkit {

class ListenerThread::Private {
public:
  FrameStorePtr store;
  Callback callback;
  ListenerPtr listener;
  size_t offset = 0;
};

ListenerThread::ListenerThread(ListenerPtr &&listener,
                               const FrameStorePtr &store,
                               const Callback &callback)
    : d(new Private()) {
  d->store = store;
  d->callback = callback;
  d->listener = (std::move(listener));
}

ListenerThread::~ListenerThread() {}

void ListenerThread::enter() {}

bool ListenerThread::loop() { return false; }

void ListenerThread::exit() {}

void ListenerThread::close() {}
}
