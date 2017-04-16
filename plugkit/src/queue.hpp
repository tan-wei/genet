#ifndef PLUGKIT_Queue_H
#define PLUGKIT_Queue_H

#include <mutex>
#include <condition_variable>
#include <queue>

namespace plugkit {

template <class T> class Queue {
public:
  Queue();
  ~Queue();
  void enqueue(T value);
  template <class It> void enqueue(It b, It e);
  template <class It> size_t dequeue(It it, size_t max);
  void close();

private:
  std::mutex mutex;
  std::condition_variable cond;
  std::queue<T> buf;
  bool closed = false;
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
  return num;
}

template <class T> void Queue<T>::close() {
  std::unique_lock<std::mutex> lock(mutex);
  closed = true;
  cond.notify_all();
}
}

#endif
