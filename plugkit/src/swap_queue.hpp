#ifndef PLUGKIT_SWAP_QUEUE_HPP
#define PLUGKIT_SWAP_QUEUE_HPP

#include <list>
#include <mutex>

template <class T>
class SwapQueue {
public:
  std::list<T> fetch();
  void push(const T &value);
  template <class... Args>
  void emplace(Args &&... args);

private:
  std::list<T> mList;
  std::mutex mMutex;
};

template <class T>
std::list<T> SwapQueue<T>::fetch() {
  std::list<T> queue;
  {
    std::lock_guard<std::mutex> lock(mMutex);
    queue.swap(mList);
  }
  return queue;
}

template <class T>
void SwapQueue<T>::push(const T &value) {
  std::lock_guard<std::mutex> lock(mMutex);
  mList.push_back(value);
}

template <class T>
template <class... Args>
void SwapQueue<T>::emplace(Args &&... args) {
  std::lock_guard<std::mutex> lock(mMutex);
  mList.emplace_back(args...);
}

#endif
