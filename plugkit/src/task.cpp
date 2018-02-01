#include "task.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace plugkit {

class TaskRunner::Private {
public:
  int count = 0;
  std::function<void(int, Task::Status)> callback;
  std::queue<std::pair<int, std::unique_ptr<Task>>> queue;

  std::mutex mutex;
  std::thread thread;
  std::condition_variable cond;
  bool closed = false;
};

TaskRunner::TaskRunner() : d(new Private()) {
  d->thread = std::thread([this]() {
    while (1) {
      std::unique_lock<std::mutex> lock(d->mutex);
      d->cond.wait(lock, [this] { return d->closed || !d->queue.empty(); });
      if (d->closed)
        return;
      int id = d->queue.front().first;
      std::unique_ptr<Task> task(std::move(d->queue.front().second));
      d->queue.pop();

      task->setCallback(
          [this, id](Task::Status status) { d->callback(id, status); });

      lock.unlock();
      task->run();
      lock.lock();
    }
  });
}

TaskRunner::~TaskRunner() { close(); }

void TaskRunner::close() {
  if (d->thread.joinable()) {
    {
      std::lock_guard<std::mutex> lock(d->mutex);
      d->closed = true;
    }
    d->cond.notify_one();
    d->thread.join();
  }
}

int TaskRunner::add(std::unique_ptr<Task> &&task) {
  int count = ++d->count;
  {
    std::lock_guard<std::mutex> lock(d->mutex);
    d->queue.emplace(count, std::move(task));
  }
  d->cond.notify_one();
  return count;
}

void TaskRunner::setCallback(
    const std::function<void(int, Task::Status)> &func) {
  std::lock_guard<std::mutex> lock(d->mutex);
  d->callback = func;
}

} // namespace plugkit
