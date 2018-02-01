#ifndef PLUGKIT_TASK_H
#define PLUGKIT_TASK_H

#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Task {
public:
  virtual void run(int id) = 0;
};

class TaskRunner final {
public:
  TaskRunner();
  ~TaskRunner();
  void close();
  int add(std::unique_ptr<Task> &&task);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
