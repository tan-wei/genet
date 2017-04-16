#ifndef PLUGKIT_STREAM_DISSECTOR_H
#define PLUGKIT_STREAM_DISSECTOR_H

#include "variant.hpp"
#include <memory>
#include <vector>
#include <regex>
#include <v8.h>

namespace plugkit {

class Layer;
using LayerPtr = std::shared_ptr<Layer>;
using LayerConstPtr = std::shared_ptr<const Layer>;

class Chunk;
using ChunkConstPtr = std::shared_ptr<const Chunk>;

struct SessionContext;

class StreamDissector {
public:
  virtual ~StreamDissector();

public:
  class Worker {
  public:
    using Timestamp = std::chrono::time_point<std::chrono::system_clock,
                                              std::chrono::nanoseconds>;

  public:
    Worker();
    virtual ~Worker();
    virtual LayerPtr analyze(const ChunkConstPtr &chunk) = 0;
    virtual bool expired(const Timestamp &lastUpdated) const;

  private:
    class Private;
    std::unique_ptr<Private> d;
  };
  using WorkerPtr = std::unique_ptr<Worker>;

public:
  virtual WorkerPtr createWorker() = 0;
  virtual std::vector<std::regex> namespaces() const = 0;
};

using StreamDissectorPtr = std::unique_ptr<StreamDissector>;

class StreamDissectorFactory;
using StreamDissectorFactoryConstPtr =
    std::shared_ptr<const StreamDissectorFactory>;

class StreamDissectorFactory {
public:
  struct TestData {
    std::vector<ChunkConstPtr> chunks;
    LayerConstPtr result;
  };

public:
  virtual ~StreamDissectorFactory();
  virtual StreamDissectorPtr create(const SessionContext &context) const = 0;
  virtual std::vector<TestData> testData() const;
  static v8::Local<v8::Object>
  wrap(const StreamDissectorFactoryConstPtr &factory);
};
}

#endif
