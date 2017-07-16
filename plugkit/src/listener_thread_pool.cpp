#include "listener_thread_pool.hpp"
#include "stream_logger.hpp"
#include "frame_store.hpp"
#include "listener.hpp"

namespace plugkit {

class ListenerThreadPool::Private {
public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  ListenerPtr listener;
  FrameStorePtr store;
  Callback callback;
};

ListenerThreadPool::ListenerThreadPool(ListenerPtr &&listener,
                                       const FrameStorePtr &store,
                                       const Callback &callback)
    : d(new Private()) {
  d->listener = std::move(listener);
  d->store = store;
  d->callback = callback;
}

ListenerThreadPool::~ListenerThreadPool() {}

void ListenerThreadPool::start() {}

void ListenerThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}
}
