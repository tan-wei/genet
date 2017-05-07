#ifndef PLUGKIT_DISSECTOR_THREAD_POOL_H
#define PLUGKIT_DISSECTOR_THREAD_POOL_H

#include <memory>
#include <functional>
#include <vector>
#include "queue.hpp"

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class Frame;
using FrameUniquePtr = std::unique_ptr<Frame>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class Variant;

class DissectorThreadPool final {
public:
  using Callback = std::function<void(FrameUniquePtr *, size_t)>;

public:
  DissectorThreadPool(const Variant &options, const Callback &callback);
  ~DissectorThreadPool();
  DissectorThreadPool(const DissectorThreadPool &) = delete;
  DissectorThreadPool &operator=(const DissectorThreadPool &) = delete;
  void start();
  void registerDissector(const DissectorFactoryConstPtr &factory);
  void setLogger(const LoggerPtr &logger);
  void push(FrameUniquePtr &&frame);
  uint32_t queue() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
