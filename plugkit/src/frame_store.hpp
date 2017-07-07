#ifndef PLUGKIT_FRAME_STORE_HPP
#define PLUGKIT_FRAME_STORE_HPP

#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace plugkit {

class Frame;
using FrameConstPtr = const Frame *;
using FrameUniquePtr = Frame *;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FrameView;
using FrameViewConstPtr = const FrameView *;

class FrameStore {
public:
  using Callback = std::function<void()>;

public:
  FrameStore(const Callback &callback);
  ~FrameStore();
  void insert(FrameUniquePtr *, size_t size);
  size_t dequeue(size_t offset, size_t max, FrameViewConstPtr *dst,
                 std::thread::id id = std::thread::id()) const;
  size_t size() const;
  std::vector<FrameViewConstPtr> get(uint32_t offset, uint32_t length) const;
  void close(std::thread::id id = std::thread::id());

private:
  FrameStore(const FrameStore &) = delete;
  FrameStore &operator=(const FrameStore &) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}
#endif
