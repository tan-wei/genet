#ifndef PLUGKIT_Queue_H
#define PLUGKIT_Queue_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace plugkit {

template <class T> class Queue final {
public:
  Queue();
  ~Queue();
  void enqueue(T value);
  template <class It> void enqueue(It b, It e);
  template <class It> size_t dequeue(It it, size_t max);
  uint32_t size() const;
  void close();

private:
  std::mutex mutex;
  std::condition_variable cond;
  std::queue<T> buf;
  bool closed = false;
  std::atomic<uint32_t> count;
};

template <class T> Queue<T>::Queue() {}

template <class T> Queue<T>::~Queue() {}

template <class T> void Queue<T>::enqueue(T value) {
  std::lock_guard<std::mutex> lock(mutex);
  if (closed)
    return;
  buf.push(std::move(value));
  cond.notify_all();
}

template <class T> template <class It> void Queue<T>::enqueue(It b, It e) {
  std::lock_guard<std::mutex> lock(mutex);
  if (closed)
    return;
  for (It it = b; it != e; ++it)
    buf.push(std::move(*it));
  count.store(buf.size(), std::memory_order_relaxed);
  cond.notify_all();
}

template <class T>
template <class It>
size_t Queue<T>::dequeue(It it, size_t max) {
  std::unique_lock<std::mutex> lock(mutex);
  cond.wait(lock, [this]() { return !buf.empty() || closed; });
  if (closed)
    return 0;
  size_t num = 0;
  while (!buf.empty() && num < max) {
    *it = std::move(buf.front());
    buf.pop();
    ++it;
    ++num;
  }
  count.store(buf.size(), std::memory_order_relaxed);
  return num;
}

template <class T> void Queue<T>::close() {
  std::unique_lock<std::mutex> lock(mutex);
  closed = true;
  cond.notify_all();
}

template <class T> uint32_t Queue<T>::size() const {
  return count.load(std::memory_order_relaxed);
}
}

#endif
