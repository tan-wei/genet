#include <nan.h>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/session_context.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_set>

using namespace plugkit;

class HTTPDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    Worker(const SessionContext &ctx) : ctx(ctx) {
      const auto array =
          ctx.options["dissector-essentials"]["httpPorts"].array();
      for (const auto &port : array) {
        ports.insert(port.uint64Value());
      }
    }

    Layer *analyze(Layer *layer) override {
      /*
      Layer child(MNS("http"));
      const auto &layer = chunk->layer();
      uint16_t srcPort =
      layer->propertyFromId(MID("src"))->value().uint64Value();
      uint16_t dstPort =
      layer->propertyFromId(MID("dst"))->value().uint64Value();
      if (ports.find(srcPort) == ports.end() && ports.find(dstPort) ==
      ports.end()) {
        return nullptr;
      }
      child.setPayload(chunk->payload());

      // TODO:ALLOC
      return new Layer(std::move(child));
      */
      return nullptr;
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
  std::vector<minins> namespaces() const override {
    return std::vector<minins>{MNS("tcp")};
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
  exports->Set(
      Nan::New("factory").ToLocalChecked(),
      StreamDissectorFactory::wrap(std::make_shared<HTTPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
