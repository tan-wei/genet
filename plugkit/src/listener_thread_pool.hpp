#ifndef PLUGKIT_LISTENER_THREAD_POOL_H
#define PLUGKIT_LISTENER_THREAD_POOL_H

#include <functional>
#include <memory>
#include <vector>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class Listener;
using ListenerPtr = std::unique_ptr<Listener>;

class ListenerThreadPool final {
public:
  using Callback = std::function<void()>;

public:
  ListenerThreadPool(ListenerPtr &&listener, const FrameStorePtr &store,
                     const Callback &callback);
  ~ListenerThreadPool();
  void start();
  void setLogger(const LoggerPtr &logger);

private:
  ListenerThreadPool(const ListenerThreadPool &) = delete;
  ListenerThreadPool &operator=(const ListenerThreadPool &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
