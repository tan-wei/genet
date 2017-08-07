#include <list>
#include <algorithm>
#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/layer.h>

using namespace plugkit;

/*
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
      end = pair.first + pair.second.length();
    }
    slices.remove_if([](const std::pair<size_t, Slice> &pair) {
      return pair.second.length() == 0;
    });
  }

  std::list<Slice> fetch() {
    std::list<Slice> sequence;
    auto it = slices.begin();
    for (; it != slices.end() && it->first == offset; ++it) {
      sequence.push_back(it->second);
      offset += it->second.length();
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
    Layer *analyze(Layer *layer) override {
      const auto &payload = layer->payload();
      uint32_t seq =
          layer->propertyFromId(Token_get("seq"))->value().uint64Value();
      uint16_t window =
          layer->propertyFromId(Token_get("window"))->value().uint64Value();
      uint8_t flags =
          layer->propertyFromId(Token_get("flags"))->value().uint64Value();
      bool syn = (flags & (0x1 << 1));

      if (syn) {
        if (currentSeq < 0) {
          currentSeq = seq;
          ring.put(receivedLength, payload);
          receivedLength += payload.length();
        }
      } else if (currentSeq >= 0) {
        if (payload.length() > 0) {
          uint64_t start = receivedLength;
          if (seq >= currentSeq) {
            start += seq - currentSeq;
            currentSeq = seq;
            ring.put(receivedLength, payload);
            receivedLength += payload.length();
          } else if (currentSeq - seq > window) {
            start += (UINT32_MAX - currentSeq) + seq;
            currentSeq = seq;
            ring.put(receivedLength, payload);
            receivedLength += payload.length();
          }
        } else if ((currentSeq + 1) % UINT32_MAX == seq) {
          currentSeq = seq;
        }
      }

      const auto &sequence = ring.fetch();
      std::vector<Variant> chunks(sequence.begin(), sequence.end());

      Property *stream = new Property(Token_get("stream"));

      Property *payloads = Layer_addProperty(child, Token_get("payloads"));
*Property_valueRef(payloads) = chunks;
      stream->addProperty(payloads);

      Property *length = Layer_addProperty(child, Token_get("length"));
*Property_valueRef(length) = receivedLength;
      stream->addProperty(length);

      if (currentSeq >= 0) {
        Property *curSeq = Layer_addProperty(child, Token_get("lastSeq"));
*Property_valueRef(curSeq) = currentSeq;
        stream->addProperty(curSeq);
      }

      layer->addProperty(stream);
      return nullptr;
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
};

class TCPStreamDissectorFactory final : public StreamDissectorFactory {
public:
  StreamDissectorPtr create(const SessionContext &ctx) const override {
    return StreamDissectorPtr(new TCPStreamDissector());
  };
};
*/

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[tcp-stream]");
  diss.type = DISSECTOR_STREAM;
  // diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
