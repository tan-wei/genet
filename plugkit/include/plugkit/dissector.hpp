#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "export.hpp"
#include "minins.hpp"
#include "types.hpp"
#include <memory>
#include <v8.h>
#include <vector>

namespace plugkit {

class PLUGKIT_EXPORT Dissector {
public:
  virtual ~Dissector();

public:
  class PLUGKIT_EXPORT Worker {
  public:
    virtual ~Worker();
    virtual Layer *analyze(Layer *layer) = 0;
    virtual const char *streamId();
  };
  using WorkerPtr = std::unique_ptr<Worker>;

public:
  virtual WorkerPtr createWorker() = 0;
  virtual std::vector<minins> namespaces() const = 0;
  virtual size_t streamIdLength() const;
};

using DissectorPtr = std::unique_ptr<Dissector>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class PLUGKIT_EXPORT DissectorFactory {
public:
  virtual ~DissectorFactory();
  virtual DissectorPtr create(const SessionContext &context) const = 0;
  static v8::Local<v8::Object> wrap(const DissectorFactoryConstPtr &factory);
};
}

#endif
