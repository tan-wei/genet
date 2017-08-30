#include <list>
#include <algorithm>
#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>

#define PLUGKIT_ENABLE_LOGGING
#include <plugkit/logger.h>

using namespace plugkit;

namespace {
const auto seqToken = Token_get("tcp.seq");
const auto windowToken = Token_get("tcp.window");
const auto flagsToken = Token_get("tcp.flags");
const auto tcpStreamToken = Token_get("tcp-stream");
const auto reassembledToken = Token_get("@reassembled");

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
        pair.second = Slice_sliceAll(pair.second, end - pair.first);
        pair.first = end;
      }
      end = pair.first + Slice_length(pair.second);
    }
    slices.remove_if([](const std::pair<size_t, Slice> &pair) {
      return Slice_length(pair.second) == 0;
    });
  }

  std::list<Slice> fetch() {
    std::list<Slice> sequence;
    auto it = slices.begin();
    for (; it != slices.end() && it->first == offset; ++it) {
      sequence.push_back(it->second);
      offset += Slice_length(it->second);
    }
    slices.erase(slices.begin(), it);
    return sequence;
  }

private:
  size_t offset = 0;
  std::list<std::pair<size_t, Slice>> slices;
};

struct Worker {
  int64_t currentSeq = -1;
  uint64_t receivedLength = 0;
  Ring ring;
};

void analyze(Context *ctx, void *data, Layer *layer) {
  Worker *worker = static_cast<Worker *>(data);

  const Slice payload = Payload_slice(Layer_payload(layer));

  uint32_t seq = Property_uint32(Layer_propertyFromId(layer, seqToken));
  uint16_t window = Property_uint32(Layer_propertyFromId(layer, windowToken));
  uint8_t flags = Property_uint32(Layer_propertyFromId(layer, flagsToken));
  bool syn = (flags & (0x1 << 1));
  if (syn) {
    if (worker->currentSeq < 0) {
      worker->currentSeq = seq;
      worker->ring.put(worker->receivedLength, payload);
      worker->receivedLength += Slice_length(payload);
    }
  } else if (worker->currentSeq >= 0) {
    if (Slice_length(payload)) {
      uint64_t start = worker->receivedLength;
      if (seq >= worker->currentSeq) {
        start += seq - worker->currentSeq;
        worker->currentSeq = seq;
        worker->ring.put(worker->receivedLength, payload);
        worker->receivedLength += Slice_length(payload);
      } else if (worker->currentSeq - seq > window) {
        start += (UINT32_MAX - worker->currentSeq) + seq;
        worker->currentSeq = seq;
        worker->ring.put(worker->receivedLength, payload);
        worker->receivedLength += Slice_length(payload);
      }
    } else if ((worker->currentSeq + 1) % UINT32_MAX == seq) {
      worker->currentSeq = seq;
    }
  }

  const auto slices = worker->ring.fetch();
  if (slices.size() > 0) {
    Payload *chunk = Layer_addPayload(layer);
    Payload_setType(chunk, reassembledToken);
    for (const auto &slice : slices) {
      Payload_addSlice(chunk, slice);
    }
  }

  Layer *sub = Layer_addSubLayer(layer, tcpStreamToken);
  Layer_addTag(sub, tcpStreamToken);
}
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_STREAM);
  Dissector_addLayerHint(diss, Token_get("tcp"));
  Dissector_setAnalyzer(diss, analyze);
  Dissector_setWorkerFactory(
      diss, [](Context *ctx) -> void * { return new Worker(); },
      [](Context *ctx, void *data) { delete static_cast<Worker *>(data); });
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
