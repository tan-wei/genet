#include "listener_thread_pool.hpp"
#include "stream_logger.hpp"
#include "frame_store.hpp"
#include "listener.hpp"

namespace plugkit {

class ListenerThreadPool::Private {
public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  ListenerFactoryConstPtr factory;
  ListenerPtr listener;
  FrameStorePtr store;
  Callback callback;
};

ListenerThreadPool::ListenerThreadPool(const ListenerFactoryConstPtr &factory,
                                       const FrameStorePtr &store,
                                       const Callback &callback)
    : d(new Private()) {
  d->factory = factory;
  d->store = store;
  d->callback = callback;
}

ListenerThreadPool::~ListenerThreadPool() {}

void ListenerThreadPool::start() {}

void ListenerThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}
}
