#ifndef PLUGKIT_FRAME_STORE_HPP
#define PLUGKIT_FRAME_STORE_HPP

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace plugkit {

class Frame;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FrameView;
class RootAllocator;

class FrameStore final {
public:
  using Callback = std::function<void()>;

public:
  FrameStore();
  ~FrameStore();
  void insert(Frame **, size_t size);
  bool dequeue(size_t offset,
               size_t *size,
               const FrameView **dst,
               std::thread::id id,
               int waitFor) const;
  size_t dequeue(size_t offset, size_t max, const Frame **dst) const;
  size_t dissectedSize() const;
  void update(uint32_t index);
  std::vector<const FrameView *> get(uint32_t offset, uint32_t length) const;
  void close(std::thread::id id = std::thread::id());
  void setCallback(const Callback &callback);
  void setAllocator(RootAllocator *allocator);

private:
  FrameStore(const FrameStore &) = delete;
  FrameStore &operator=(const FrameStore &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
} // namespace plugkit
#endif
