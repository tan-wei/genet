#ifndef PLUGKIT_DISSECTOR_THREAD_H
#define PLUGKIT_DISSECTOR_THREAD_H

#include "queue.hpp"
#include "worker_thread.hpp"
#include <memory>
#include <vector>

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
  struct CallbackData {
    Frame **frames;
    size_t frameSize;
    std::pair<Layer *, std::string> *layers;
    size_t layerSize;
  };
  using StreamIdMap = std::vector<std::pair<Layer *, std::string>>;
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
