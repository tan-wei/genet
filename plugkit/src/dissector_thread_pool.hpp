#ifndef PLUGKIT_DISSECTOR_THREAD_POOL_H
#define PLUGKIT_DISSECTOR_THREAD_POOL_H

#include "queue.hpp"
#include "variant_map.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class Frame;

struct Dissector;
class RootAllocator;

class DissectorThreadPool final {
public:
  using Callback = std::function<void(Frame **, size_t)>;
  using InspectorCallback =
      std::function<void(const std::string &id, const std::string &msg)>;

public:
  DissectorThreadPool();
  ~DissectorThreadPool();
  DissectorThreadPool(const DissectorThreadPool &) = delete;
  DissectorThreadPool &operator=(const DissectorThreadPool &) = delete;
  void start();
  void registerDissector(const Dissector &diss);
  void setOptions(const VariantMap &options);
  void setCallback(const Callback &callback);
  void setLogger(const LoggerPtr &logger);
  void setAllocator(RootAllocator *allocator);
  void push(Frame **begin, size_t length);

  void sendInspectorMessage(const std::string &id, const std::string &msg);
  void setInspectorCallback(const InspectorCallback &callback);
  std::vector<std::string> inspectors() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
} // namespace plugkit

#endif
