#ifndef PLUGKIT_STREAM_DISSECTOR_THREAD_H
#define PLUGKIT_STREAM_DISSECTOR_THREAD_H

#include "worker_thread.hpp"
#include <memory>

namespace plugkit {

class Frame;
using FrameUniquePtr = std::unique_ptr<Frame>;

class Chunk;
using ChunkConstPtr = std::shared_ptr<const Chunk>;

class StreamDissectorFactory;
using StreamDissectorFactoryConstPtr =
    std::shared_ptr<const StreamDissectorFactory>;

class Variant;

class StreamDissectorThread final : public WorkerThread {
public:
  using Callback = std::function<void(FrameUniquePtr *, size_t)>;

public:
  StreamDissectorThread(const Variant &options, const Callback &callback);
  ~StreamDissectorThread() override;
  void
  pushStreamDissectorFactory(const StreamDissectorFactoryConstPtr &factory);
  void enter() override;
  bool loop() override;
  void exit() override;
  void push(const ChunkConstPtr *begin, size_t size);
  void stop();

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
