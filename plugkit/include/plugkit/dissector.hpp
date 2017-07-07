#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "variant.hpp"
#include "export.hpp"
#include "strns.hpp"
#include <memory>
#include <v8.h>
#include <vector>

namespace plugkit {

class Layer;
using LayerPtr = std::shared_ptr<Layer>;
using LayerConstPtr = std::shared_ptr<const Layer>;

class Frame;
using FrameConstPtr = std::shared_ptr<const Frame>;

struct SessionContext;

class PLUGKIT_EXPORT Dissector {
public:
  virtual ~Dissector();

public:
  class PLUGKIT_EXPORT Worker {
  public:
    virtual ~Worker();
    virtual LayerPtr analyze(const LayerConstPtr &layer) = 0;
  };
  using WorkerPtr = std::unique_ptr<Worker>;

public:
  virtual WorkerPtr createWorker() = 0;
  virtual std::vector<strns> namespaces() const = 0;
};

using DissectorPtr = std::unique_ptr<Dissector>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class PLUGKIT_EXPORT DissectorFactory {
public:
  struct TestData {
    LayerConstPtr parent;
    LayerConstPtr result;
  };

public:
  virtual ~DissectorFactory();
  virtual DissectorPtr create(const SessionContext &context) const = 0;
  virtual std::vector<TestData> testData() const;
  static v8::Local<v8::Object> wrap(const DissectorFactoryConstPtr &factory);
};
}

#endif
