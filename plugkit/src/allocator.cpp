#include "allocator.hpp"

namespace plugkit {
RootAllocator::RootAllocator() {}

RootAllocator::~RootAllocator() {
  for (char *ptr : mList) {
    delete[] ptr;
  }
}

void *RootAllocator::allocate(size_t size) {
  char *ptr = new char[size];
  std::lock_guard<std::mutex> lock(mMutex);
  mList.push_back(ptr);
  return ptr;
}
} // namespace plugkit
