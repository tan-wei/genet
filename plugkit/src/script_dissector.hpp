#ifndef PLUGKIT_SCRIPT_DISSECTOR_H
#define PLUGKIT_SCRIPT_DISSECTOR_H

#include "dissector.hpp"
#include <string>
#include <v8.h>

namespace plugkit {

struct SessionContext;

class ScriptDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
    friend class ScriptDissector;

  public:
    Worker(const SessionContext &ctx,
           const v8::UniquePersistent<v8::Object> &workerObj);
    ~Worker();
    Layer *analyze(Layer *layer) override;

  private:
    class Private;
    std::unique_ptr<Private> d;
  };

public:
  ScriptDissector(const SessionContext &ctx, const std::string &script,
                  const std::string &path);
  ~ScriptDissector();
  WorkerPtr createWorker() override;
  std::vector<minins> namespaces() const override;

private:
  class Private;
  std::unique_ptr<Private> d;
};

class ScriptDissectorFactory final : public DissectorFactory {
public:
  ScriptDissectorFactory(const std::string &script, const std::string &path);
  ~ScriptDissectorFactory();
  DissectorPtr create(const SessionContext &context) const override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
