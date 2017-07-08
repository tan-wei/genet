#ifndef PLUGKIT_MEMORY_POOL_HPP
#define PLUGKIT_MEMORY_POOL_HPP
#include <list>

namespace plugkit {

template <class T, class P, size_t initSize = 2048, int maxScale = 5>
class MemoryPool {
public:
  MemoryPool();
  ~MemoryPool();
  void *alloc();

private:
  std::list<char *> pool;
  size_t used;
  size_t block;
  int scale;
};

template <class T, class P, size_t initSize, int maxScale>
MemoryPool<T, initSize, maxScale>::MemoryPool()
    : used(0), block(initSize), scale(1) {}

template <class T, class P, size_t initSize, int maxScale>
MemoryPool<T, initSize, maxScale>::~MemoryPool() {
  for (char *ptr : pool) {
    delete[] ptr;
  }
}

template <class T, class P, size_t initSize, int maxScale>
void *MemoryPool<T, initSize, maxScale>::alloc() {
  using Pair = std::pair<T, P>;
  if (pool.empty()) {
    pool.push_back(new char[block * sizeof(Pair)]);
  } else if (used >= block) {
    if (scale < maxScale) {
      ++scale;
      block *= 2;
    }
    pool.push_back(new char[block * sizeof(Pair)]);
    used = 0;
  }
  void *ptr = pool.back() + used * sizeof(Pair);
  ++used;
  return ptr;
}
}

#endif
