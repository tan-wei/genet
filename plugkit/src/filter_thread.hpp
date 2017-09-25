#ifndef PLUGKIT_FILTER_THREAD_H
#define PLUGKIT_FILTER_THREAD_H

#include "worker_thread.hpp"
#include <memory>

namespace plugkit {

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FilterThread final : public WorkerThread {
public:
  using Callback =
      std::function<void(uint32_t begin, const std::vector<char> &)>;

public:
  FilterThread(const std::string &body, const FrameStorePtr &store,
               const Callback &callback);
  ~FilterThread() override;
  void enter() override;
  bool loop() override;
  void exit() override;
  void close();

private:
  class Private;
  std::unique_ptr<Private> d;
};
} // namespace plugkit

#endif
