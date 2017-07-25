#ifndef PLUGKIT_DISSECTOR_THREAD_H
#define PLUGKIT_DISSECTOR_THREAD_H

#include "queue.hpp"
#include "worker_thread.hpp"
#include <memory>
#include <unordered_map>

namespace plugkit {

class Frame;

using FrameQueue = Queue<Frame *>;
using FrameQueuePtr = std::shared_ptr<FrameQueue>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class Layer;
class Variant;

class DissectorThread final : public WorkerThread {
public:
  using StreamIdMap =
      std::unordered_map<Layer *, std::pair<const char *, size_t>>;
  using Callback = std::function<void(Frame **, size_t)>;

public:
  DissectorThread(const Variant &options, const FrameQueuePtr &queue,
                  const Callback &callback);
  ~DissectorThread() override;
  void pushDissectorFactory(const DissectorFactoryConstPtr &factory);
  void enter() override;
  bool loop() override;
  void exit() override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
