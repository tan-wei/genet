#ifndef PLUGKIT_FILTER_THREAD_POOL_H
#define PLUGKIT_FILTER_THREAD_POOL_H

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

namespace plugkit {

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FilterThreadPool final {
public:
  using Callback = std::function<void()>;
  using AliasMap = std::unordered_map<std::string, std::string>;

public:
  FilterThreadPool(const std::string &body, const FrameStorePtr &store,
                   const AliasMap &map, const Callback &callback);
  ~FilterThreadPool();
  void start();
  void setLogger(const LoggerPtr &logger);

  std::vector<uint32_t> get(uint32_t offset, uint32_t length) const;
  uint32_t size() const;
  uint32_t maxSeq() const;

private:
  FilterThreadPool(const FilterThreadPool &) = delete;
  FilterThreadPool &operator=(const FilterThreadPool &) = delete;

private:
  class Private;
  Private *d;
};
}

#endif
