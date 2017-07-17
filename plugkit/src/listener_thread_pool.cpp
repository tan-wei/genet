#include "listener_thread_pool.hpp"
#include "listener_thread.hpp"
#include "stream_logger.hpp"
#include "session_context.hpp"
#include "frame_store.hpp"
#include "listener.hpp"

namespace plugkit {

class ListenerThreadPool::Private {
public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  ListenerFactoryConstPtr factory;
  std::unique_ptr<ListenerThread> thread;
  FrameStorePtr store;
  Callback callback;
  Variant args;
};

ListenerThreadPool::ListenerThreadPool(const ListenerFactoryConstPtr &factory,
                                      const Variant& args,
                                       const FrameStorePtr &store,
                                       const Callback &callback)
    : d(new Private()) {
  d->factory = factory;
  d->args = args;
  d->store = store;
  d->callback = callback;
}

ListenerThreadPool::~ListenerThreadPool() {
  if (d->thread) {
    d->thread->close();
  }
}

void ListenerThreadPool::start() {
  auto listener = d->factory->create(d->args, SessionContext());
  auto thread = new ListenerThread(std::move(listener), d->store, d->callback);
  thread->setLogger(d->logger);
  thread->start();
  d->thread.reset(std::move(thread));
}

void ListenerThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}
}
