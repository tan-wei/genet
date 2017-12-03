#ifndef PLUGKIT_DISSECTOR_THREAD_H
#define PLUGKIT_DISSECTOR_THREAD_H

#include "queue.hpp"
#include "worker_thread.hpp"
#include <unordered_map>

namespace plugkit {

class Frame;

using FrameQueue = Queue<Frame *>;
using FrameQueuePtr = std::shared_ptr<FrameQueue>;

class StreamResolver;
using StreamResolverPtr = std::shared_ptr<StreamResolver>;

struct Variant;
using OptionMap = std::unordered_map<std::string, Variant>;

struct Dissector;

class DissectorThread final : public WorkerThread {
public:
  using Callback = std::function<void(Frame **, size_t)>;

public:
  DissectorThread(const OptionMap &options,
                  const FrameQueuePtr &queue,
                  const Callback &callback);
  ~DissectorThread() override;
  void pushDissector(const Dissector &diss);
  void enter() override;
  bool loop() override;
  void exit() override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
} // namespace plugkit

#endif
