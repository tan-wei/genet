#ifndef PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H
#define PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H

#include "queue.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class Frame;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class Variant;

struct Dissector;

class StreamDissectorThreadPool final {
public:
  using Callback = std::function<void(uint32_t)>;

public:
  StreamDissectorThreadPool(const Variant &options, const FrameStorePtr &store,
                            const Callback &callback);
  ~StreamDissectorThreadPool();
  void registerDissector(const Dissector &diss);
  void start();
  void setLogger(const LoggerPtr &logger);

private:
  StreamDissectorThreadPool(const StreamDissectorThreadPool &) = delete;
  StreamDissectorThreadPool &
  operator=(const StreamDissectorThreadPool &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
