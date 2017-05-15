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
    LayerPtr analyze(const ChunkConstPtr &chunk) override {
      Layer child;
      const auto& layer = chunk->layer();
      const auto& payload = chunk->payload();
      uint32_t seq = layer->propertyFromId("seq")->value().uint64Value();
      uint8_t flags = layer->propertyFromId("flags")->value().uint64Value();
      bool syn = (flags & (0x1 << 1));

      Slice chunkPayload;
      if (syn) {
        seq_ = seq;
        length_ += payload.size();
        chunkPayload = payload;
      }

      if (payload.size() > 0) {
        if (seq_ < 0) {
          length_ += payload.size();
          chunkPayload = payload;
        }
      }
      seq_ = seq;

      if (chunkPayload.size() > 0) {
        child.addChunk(Chunk(fmt::replace(chunk->streamNs(), "<tcp>", "tcp"),
          chunk->streamId(), chunkPayload));
      }
      return std::make_shared<Layer>(std::move(child));
    }

  private:
    int64_t seq_ = -1;
    uint64_t length_ = 0;
  };

public:
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new TCPStreamDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("eth ipv4 <tcp>$")};
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
