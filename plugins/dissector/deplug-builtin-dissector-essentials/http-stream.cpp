#include <nan.h>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/session_context.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_set>

using namespace plugkit;

class HTTPDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    Worker(const SessionContext &ctx) : ctx(ctx) {
      const auto array =
        ctx.options["deplug-builtin-dissector-essentials"]["httpPorts"].array();
      for (const auto& port : array) {
        ports.insert(port.uint64Value());
      }
    }

    LayerPtr analyze(const ChunkConstPtr &chunk) override {
      Layer child(chunk->streamNs() + " http");
      const auto &layer = chunk->layer();
      uint16_t srcPort = layer->propertyFromId(PK_STRID("src"))->value().uint64Value();
      uint16_t dstPort = layer->propertyFromId(PK_STRID("dst"))->value().uint64Value();
      if (ports.find(srcPort) == ports.end() && ports.find(dstPort) == ports.end()) {
        return LayerPtr();
      }
      child.setPayload(chunk->payload());
      return std::make_shared<Layer>(std::move(child));
    }

  private:
    SessionContext ctx;
    std::unordered_set<uint16_t> ports;
  };

public:
  HTTPDissector(const SessionContext &ctx) : ctx(ctx) {}
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new HTTPDissector::Worker(ctx));
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("tcp$")};
  }

private:
  SessionContext ctx;
};

class HTTPDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext &ctx) const override {
    return StreamDissectorPtr(new HTTPDissector(ctx));
  };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
               StreamDissectorFactory::wrap(
                   std::make_shared<HTTPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
