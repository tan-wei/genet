#ifndef PLUGKIT_SCRIPT_STREAM_DISSECTOR_H
#define PLUGKIT_SCRIPT_STREAM_DISSECTOR_H

#include "stream_dissector.hpp"
#include <string>
#include <v8.h>

namespace plugkit {

struct SessionContext;

class ScriptStreamDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
    friend class ScriptStreamDissector;

  public:
    Worker(const SessionContext &ctx,
           const v8::UniquePersistent<v8::Object> &workerObj);
    ~Worker();
    Layer *analyze(Layer *layer) override;
    bool expired(const Timestamp &lastUpdated) const override;

  private:
    class Private;
    std::unique_ptr<Private> d;
  };

public:
  ScriptStreamDissector(const SessionContext &ctx, const std::string &script,
                        const std::string &path);
  ~ScriptStreamDissector();
  WorkerPtr createWorker() override;

private:
  class Private;
  std::unique_ptr<Private> d;
};

class ScriptStreamDissectorFactory final : public StreamDissectorFactory {
public:
  ScriptStreamDissectorFactory(const std::string &script,
                               const std::string &path);
  ~ScriptStreamDissectorFactory();
  StreamDissectorPtr create(const SessionContext &context) const override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
