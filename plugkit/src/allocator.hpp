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
  T *allocUninitialized(size_t size);
  bool allocable(T *ptr) const;
  void dealloc(T *ptr);

private:
  static const size_t blockSize = 1024;
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
  T *ptr = allocUninitialized(1);
  return new (ptr) T(args...);
}

template <class T>
T *BlockAllocator<T>::allocUninitialized(size_t size) {
  if (size == 0 || size >= blockSize - 1)
    return nullptr;
  Block *chunk = nullptr;
  if (!mList.empty()) {
    Block *front = mList.front().first;
    chunk = front->next;
    for (size_t index = 0; index < size; index += 1) {
      if (index > 0 && front->next != front + 1) {
        chunk = nullptr;
        break;
      }
      front = front->next;
    }
  }
  if (chunk) {
    Block *front = mList.front().first;
    for (size_t index = 0; index < size; index += 1) {
      front->next = front->next->next;
    }
    return reinterpret_cast<T *>(chunk->data);
  }
  Block *front =
      static_cast<Block *>(mRoot->allocate(sizeof(Block) * blockSize));
  chunk = front + 1;
  front->next = chunk + size;
  for (size_t i = 1 + size; i < blockSize - 1; ++i) {
    front[i].next = front + i + 1;
  }
  front[blockSize - 1].next = nullptr;
  mList.emplace_front(front, front + blockSize);
  return reinterpret_cast<T *>(chunk->data);
}

template <class T>
bool BlockAllocator<T>::allocable(T *ptr) const {
  return !mList.empty() && mList.front().first->next == ptr;
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
