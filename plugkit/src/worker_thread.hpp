#ifndef PLUGKIT_WORKER_THREAD_H
#define PLUGKIT_WORKER_THREAD_H

#include <thread>

namespace plugkit {

class SessionContext;

class WorkerThread {
public:
  using InspectorCallback = std::function<void(const std::string &)>;

public:
  WorkerThread(const SessionContext *sctx);
  virtual ~WorkerThread();
  WorkerThread(const WorkerThread &) = delete;
  WorkerThread &operator=(const WorkerThread &) = delete;
  virtual void enter() = 0;
  virtual bool loop() = 0;
  virtual void exit() = 0;
  void start();
  void join();

  void sendInspectorMessage(const std::string &msg);
  void setInspector(const std::string &id, const InspectorCallback &callback);

protected:
  bool inspectorActivated() const;

protected:
  std::thread thread;

private:
  class Private;
  std::unique_ptr<Private> d;

  class ArrayBufferAllocator;
  class InspectorClient;
  class InspectorChannel;
};
} // namespace plugkit

#endif
