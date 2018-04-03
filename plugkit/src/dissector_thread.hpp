#ifndef PLUGKIT_DISSECTOR_THREAD_H
#define PLUGKIT_DISSECTOR_THREAD_H

#include "config_map.hpp"
#include "queue.hpp"
#include "worker_thread.hpp"

namespace plugkit {

class Frame;
class RootAllocator;
class SessionContext;

using FrameQueue = Queue<Frame *>;
using FrameQueuePtr = std::shared_ptr<FrameQueue>;

class StreamResolver;
using StreamResolverPtr = std::shared_ptr<StreamResolver>;

struct Dissector;

class DissectorThread final : public WorkerThread {
public:
  using Callback = std::function<void(Frame **, size_t)>;

public:
  DissectorThread(const SessionContext *sctx,
                  const ConfigMap &options,
                  const FrameQueuePtr &queue,
                  const Callback &callback);
  ~DissectorThread() override;
  void pushDissector(const Dissector &diss);
  void setAllocator(RootAllocator *allocator);
  void enter() override;
  bool loop() override;
  void exit() override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
} // namespace plugkit

#endif
