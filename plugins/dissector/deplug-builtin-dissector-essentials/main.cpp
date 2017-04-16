#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>

using namespace plugkit;

class MyDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      const LayerPtr& child = std::make_shared<Layer>();
      child->setNs("eth");
      child->setName("Ethernet");
      return child ;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new MyDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<eth>")};
  }
};

class MyStreamDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    LayerPtr analyze(const ChunkConstPtr &layer) override {
      const LayerPtr& child = std::make_shared<Layer>();
      child->setNs("ethk");
      child->setName("Ethernetl");
      return child ;
    }
  };

public:
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new MyStreamDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("eth ipv4 tcp")};
  }
};

class MyDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override { return DissectorPtr(new MyDissector()); };
};

class MyStreamDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext& ctx) const override { return StreamDissectorPtr(new MyStreamDissector()); };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(), DissectorFactory::wrap(std::make_shared<MyDissectorFactory>()));
  exports->Set(Nan::New("streamFactory").ToLocalChecked(), StreamDissectorFactory::wrap(std::make_shared<MyStreamDissectorFactory>()));
}

NODE_MODULE(plugkitNativePluginExample, Init)
