#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class TCPDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer child(fmt::replace(layer->ns(), "<tcp>", "tcp"), "TCP");
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new TCPDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<tcp>$")};
  }
};

class TCPDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new TCPDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<TCPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
