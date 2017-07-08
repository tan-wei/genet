#ifndef PLUGKIT_STREAM_DISSECTOR_THREAD_H
#define PLUGKIT_STREAM_DISSECTOR_THREAD_H

#include "worker_thread.hpp"
#include <memory>
#include <vector>

namespace plugkit {

class Frame;
class Chunk;

class StreamDissectorFactory;
using StreamDissectorFactoryConstPtr =
    std::shared_ptr<const StreamDissectorFactory>;

class Variant;

class StreamDissectorThread final : public WorkerThread {
public:
  using Callback = std::function<void(Frame **, size_t)>;

public:
  StreamDissectorThread(const Variant &options, const Callback &callback);
  ~StreamDissectorThread() override;
  void
  pushStreamDissectorFactory(const StreamDissectorFactoryConstPtr &factory);
  void enter() override;
  bool loop() override;
  void exit() override;
  void push(const Chunk *const *begin, size_t size);
  void stop();
  uint32_t queue() const;

private:
  std::vector<const Chunk *> processChunk(const Chunk *chunk);

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
