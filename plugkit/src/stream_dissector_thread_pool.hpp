#ifndef PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H
#define PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H

#include "queue.hpp"
#include "variant_map.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class Frame;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

struct Dissector;
class RootAllocator;

class StreamDissectorThreadPool final {
public:
  using Callback = std::function<void(uint32_t)>;

public:
  StreamDissectorThreadPool();
  ~StreamDissectorThreadPool();
  void registerDissector(const Dissector &diss);
  void setOptions(const VariantMap &options);
  void setFrameStore(const FrameStorePtr &store);
  void setCallback(const Callback &callback);
  void setAllocator(RootAllocator *allocator);
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
} // namespace plugkit

#endif
