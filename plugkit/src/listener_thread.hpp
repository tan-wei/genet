#ifndef PLUGKIT_LISTENER_THREAD_H
#define PLUGKIT_LISTENER_THREAD_H

#include "worker_thread.hpp"
#include <memory>

namespace plugkit {

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class Listener;
using ListenerPtr = std::unique_ptr<Listener>;

class ListenerThread final : public WorkerThread {
public:
  using Callback = std::function<void()>;

public:
  ListenerThread(ListenerPtr &&listener, const FrameStorePtr &store,
                 const Callback &callback);
  ~ListenerThread() override;
  void enter() override;
  bool loop() override;
  void exit() override;
  void close();

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
