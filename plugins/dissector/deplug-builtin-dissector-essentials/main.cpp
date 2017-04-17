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

class MyDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override { return DissectorPtr(new MyDissector()); };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(), DissectorFactory::wrap(std::make_shared<MyDissectorFactory>()));
}

NODE_MODULE(plugkitNativePluginExample, Init)
