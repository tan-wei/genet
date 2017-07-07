#include <list>
#include <algorithm>
#include <nan.h>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/fmt.hpp>

using namespace plugkit;

class Ring {
public:
  Ring() {}
  ~Ring() {}

  void put(size_t pos, const Slice &slice) {
    auto lower = std::lower_bound(
        slices.begin(), slices.end(), std::make_pair(pos, Slice()),
        [](const std::pair<size_t, Slice> &a,
           const std::pair<size_t, Slice> &b) { return a.first < b.first; });
    slices.insert(lower, std::make_pair(pos, slice));
    size_t end = offset;
    for (auto &pair : slices) {
      if (pair.first < end) {
        pair.second = pair.second.slice(end - pair.first);
        pair.first = end;
      }
      end = pair.first + pair.second.size();
    }
    slices.remove_if([](const std::pair<size_t, Slice> &pair) {
      return pair.second.size() == 0;
    });
  }

  std::list<Slice> fetch() {
    std::list<Slice> sequence;
    auto it = slices.begin();
    for (; it != slices.end() && it->first == offset; ++it) {
      sequence.push_back(it->second);
      offset += it->second.size();
    }
    slices.erase(slices.begin(), it);
    return sequence;
  }

private:
  size_t offset = 0;
  std::list<std::pair<size_t, Slice>> slices;
};

class TCPStreamDissector final : public StreamDissector {
public:
  class Worker final : public StreamDissector::Worker {
  public:
    LayerPtr analyze(const ChunkConstPtr &chunk) override {
      Layer child;
      const auto &layer = chunk->layer();
      const auto &payload = chunk->payload();
      uint32_t seq = layer->propertyFromId(PK_STRID("seq"))->value().uint64Value();
      uint16_t window = layer->propertyFromId(PK_STRID("window"))->value().uint64Value();
      uint8_t flags = layer->propertyFromId(PK_STRID("flags"))->value().uint64Value();
      bool syn = (flags & (0x1 << 1));

      if (syn) {
        if (currentSeq < 0) {
          currentSeq = seq;
          ring.put(receivedLength, payload);
          receivedLength += payload.size();
        }
      } else if (currentSeq >= 0) {
        if (payload.size() > 0) {
          uint64_t start = receivedLength;
          if (seq >= currentSeq) {
            start += seq - currentSeq;
            currentSeq = seq;
            ring.put(receivedLength, payload);
            receivedLength += payload.size();
          } else if (currentSeq - seq > window) {
            start += (UINT32_MAX - currentSeq) + seq;
            currentSeq = seq;
            ring.put(receivedLength, payload);
            receivedLength += payload.size();
          }
        } else if ((currentSeq + 1) % UINT32_MAX == seq) {
          currentSeq = seq;
        }
      }

      const auto &sequence = ring.fetch();
      for (const auto& slice : sequence) {
        Chunk subChunk(PK_STRNS("tcp"), chunk->streamId(), slice);
        child.addChunk(std::move(subChunk));
      }

      // TODO:ALLOC
      return new Layer(std::move(child));
    }

  private:
    int64_t currentSeq = -1;
    uint64_t receivedLength = 0;
    Ring ring;
  };

public:
  StreamDissector::WorkerPtr createWorker() override {
    return StreamDissector::WorkerPtr(new TCPStreamDissector::Worker());
  }
  std::vector<strns> namespaces() const override {
    return std::vector<strns>{PK_STRNS("*tcp")};
  }
};

class TCPStreamDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext &ctx) const override {
    return StreamDissectorPtr(new TCPStreamDissector());
  };
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
               StreamDissectorFactory::wrap(
                   std::make_shared<TCPStreamDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
