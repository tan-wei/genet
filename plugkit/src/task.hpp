#ifndef PLUGKIT_TASK_H
#define PLUGKIT_TASK_H

#include <functional>
#include <memory>

namespace plugkit {

class Task {
public:
  struct Status {
    double progress = 0.0;
  };

public:
  virtual void run() = 0;
  virtual void setCallback(const std::function<void(Status)> &func) = 0;
};

class TaskRunner final {
public:
  TaskRunner();
  ~TaskRunner();
  void close();
  int add(std::unique_ptr<Task> &&task);
  void setCallback(const std::function<void(int, Task::Status)> &func);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
