#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class TCPStreamDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    LayerPtr analyze(const ChunkConstPtr &layer) override {
      const LayerPtr& child = std::make_shared<Layer>();
      return child;
    }
  };

public:
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new TCPStreamDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("eth ipv4 <tcp>")};
  }
};

class TCPStreamDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext& ctx) const override {
    return StreamDissectorPtr(new TCPStreamDissector());
  };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    StreamDissectorFactory::wrap(std::make_shared<TCPStreamDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
