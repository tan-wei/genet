#ifndef PLUGKIT_ALLOCATOR_HPP
#define PLUGKIT_ALLOCATOR_HPP

#include <list>
#include <mutex>

namespace plugkit {
class RootAllocator final {
public:
  RootAllocator();
  ~RootAllocator();
  void *allocate(size_t size);

private:
  std::list<char *> mList;
  std::mutex mMutex;
};

template <class T>
class BlockAllocator final {
public:
  BlockAllocator(RootAllocator *root);
  ~BlockAllocator();
  template <class... Args>
  T *alloc(Args... args);
  void dealloc(T *ptr);

private:
  union Block {
    Block *next;
    char data[sizeof(T)];
  };

  RootAllocator *mRoot;
  std::list<std::pair<Block *, Block *>> mList;
};

template <class T>
BlockAllocator<T>::BlockAllocator(RootAllocator *root) : mRoot(root) {}

template <class T>
BlockAllocator<T>::~BlockAllocator() {}

template <class T>
template <class... Args>
T *BlockAllocator<T>::alloc(Args... args) {
  Block *chunk = nullptr;
  for (auto pair : mList) {
    Block *front = pair.first;
    if (front->next) {
      chunk = front->next;
      front->next = front->next->next;
      break;
    }
  }
  if (!chunk) {
    size_t length = 1024;
    Block *front =
        static_cast<Block *>(mRoot->allocate(sizeof(Block) * length));
    chunk = front + 1;
    front->next = front + 2;
    for (size_t i = 2; i < length - 1; ++i) {
      front[i].next = front + i + 1;
    }
    front[length - 1].next = nullptr;
    mList.push_front(std::make_pair(front, front + length));
  }
  return new (chunk) T(args...);
}

template <class T>
void BlockAllocator<T>::dealloc(T *ptr) {
  for (auto pair : mList) {
    Block *front = pair.first;
    Block *chunk = reinterpret_cast<Block *>(ptr);
    if (front < chunk && chunk < pair.second) {
      ptr->~T();
      chunk->next = front->next;
      front->next = chunk;
      break;
    }
  }
}
} // namespace plugkit

#endif
