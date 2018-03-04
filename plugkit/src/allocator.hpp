#ifndef PLUGKIT_ALLOCATOR_HPP
#define PLUGKIT_ALLOCATOR_HPP

#include <cstring>
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
  static_assert(sizeof(Block) == sizeof(T),
                "sizeof T must be larger than the pointer size");

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
  if (size == 0 || size > blockSize - 1)
    return nullptr;
  Block *chunk = nullptr;
  if (!mList.empty()) {
    Block *front = mList.front().first;
    if ((chunk = front->next)) {
      for (size_t index = 0; index < size; index += 1) {
        if (index > 0 && front->next != front + 1) {
          chunk = nullptr;
          break;
        }
        front = front->next;
      }
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
  return !mList.empty() &&
         reinterpret_cast<T *>(mList.front().first->next) == ptr;
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

template <class T>
class BlockVector final {
public:
  BlockVector();
  ~BlockVector();
  template <class... Args>
  void emplace_back(BlockAllocator<T> *alloc, Args... args);
  T *data() const;
  T &operator[](size_t index);
  const T &operator[](size_t index) const;
  bool empty() const;
  size_t size() const;

private:
  union Item {
    uint32_t size;
    T data;
  };
  static_assert(sizeof(Item) == sizeof(T),
                "sizeof T must be larger than 32bit");
  Item *mBegin;
};

template <class T>
BlockVector<T>::BlockVector() : mBegin(nullptr) {}

template <class T>
BlockVector<T>::~BlockVector() {}

template <class T>
template <class... Args>
void BlockVector<T>::emplace_back(BlockAllocator<T> *alloc, Args... args) {
  if (mBegin) {
    if (alloc->allocable(&mBegin->data + mBegin->size + 1)) {
      alloc->alloc(args...);
    } else {
      Item *begin =
          reinterpret_cast<Item *>(alloc->allocUninitialized(mBegin->size + 2));
      std::memcpy(begin, mBegin, sizeof(Item) * (mBegin->size + 1));
      new (&begin[mBegin->size + 1].data) T(args...);
      mBegin = begin;
    }
  } else {
    mBegin = reinterpret_cast<Item *>(alloc->allocUninitialized(2));
    new (&mBegin[1].data) T(args...);
    mBegin->size = 0;
  }
  mBegin->size += 1;
}

template <class T>
T *BlockVector<T>::data() const {
  if (mBegin) {
    return &mBegin->data + 1;
  }
  return nullptr;
}

template <class T>
T &BlockVector<T>::operator[](size_t index) {
  return data()[index];
}

template <class T>
const T &BlockVector<T>::operator[](size_t index) const {
  return data()[index];
}

template <class T>
bool BlockVector<T>::empty() const {
  return size() == 0;
}

template <class T>
size_t BlockVector<T>::size() const {
  if (mBegin) {
    return mBegin->size;
  }
  return 0;
}

} // namespace plugkit

#endif
