#ifndef PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H
#define PLUGKIT_STREAM_DISSECTOR_THREAD_POOL_H

#include "config_map.hpp"
#include "queue.hpp"
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
class SessionContext;

class StreamDissectorThreadPool final {
public:
  using Callback = std::function<void(uint32_t)>;
  using InspectorCallback =
      std::function<void(const std::string &id, const std::string &msg)>;

public:
  StreamDissectorThreadPool(const SessionContext *sctx);
  ~StreamDissectorThreadPool();
  void registerDissector(const Dissector &diss);
  void setConfig(const ConfigMap &options);
  void setFrameStore(const FrameStorePtr &store);
  void setCallback(const Callback &callback);
  void setAllocator(RootAllocator *allocator);
  void start();

  void sendInspectorMessage(const std::string &id, const std::string &msg);
  void setInspectorCallback(const InspectorCallback &callback);
  std::vector<std::string> inspectors() const;

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
