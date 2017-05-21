#include <nan.h>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/fmt.hpp>

using namespace plugkit;

class HTTPDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    LayerPtr analyze(const ChunkConstPtr &chunk) override {
      Layer child(chunk->streamNs() + " http", "HTTP");
      const auto &layer = chunk->layer();
      uint16_t srcPort = layer->propertyFromId("src")->value().uint64Value();
      uint16_t dstPort = layer->propertyFromId("dst")->value().uint64Value();
      if (srcPort != 80 && dstPort != 80) {
        return LayerPtr();
      }
      child.setPayload(chunk->payload());
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new HTTPDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("tcp$")};
  }
};

class HTTPDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext &ctx) const override {
    return StreamDissectorPtr(new HTTPDissector());
  };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
               StreamDissectorFactory::wrap(
                   std::make_shared<HTTPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
