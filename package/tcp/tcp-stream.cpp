#include <algorithm>
#include <list>
#include <nan.h>
#include <plugkit/attr.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <plugkit/token.h>
#include <plugkit/variant.h>
#include <unordered_map>

using namespace plugkit;

using StreamID = std::tuple<uint16_t, uint16_t, std::string, std::string>;

namespace std {
template <>
struct hash<StreamID> {
public:
  size_t operator()(const StreamID &id) const {
    return hash<uint16_t>()(std::get<0>(id)) ^
           hash<uint16_t>()(std::get<1>(id)) ^
           hash<std::string>()(std::get<2>(id)) ^
           hash<std::string>()(std::get<3>(id));
  }
};
} // namespace std

namespace {
const auto streamIdToken = Token_get("tcp.streamId");
const auto seqToken = Token_get("tcp.seq");
const auto windowToken = Token_get("tcp.window");
const auto flagsToken = Token_get("tcp.flags");
const auto tcpStreamToken = Token_get("tcp-stream");
const auto streamToken = Token_get("@stream");

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

struct Stream {
  uint32_t id = 0;
  int64_t currentSeq = -1;
  uint64_t receivedLength = 0;
  Ring ring;
};

struct TCPWorker {
  std::unordered_map<StreamID, Stream> idMap;
};

void analyze(Context *ctx, const Dissector *diss, Worker data, Layer *layer) {
  TCPWorker *worker = static_cast<TCPWorker *>(data.data);

  const auto parentLayerId = Layer_id(Layer_parent(layer));
  const auto layerId = Layer_id(layer);

  const auto &parentSrc = Attr_slice(Layer_attr(Layer_parent(layer), Token_join(parentLayerId, ".src")));
  const auto &parentDst = Attr_slice(Layer_attr(Layer_parent(layer), Token_join(parentLayerId, ".dst")));

  const auto &id =
      StreamID(Attr_uint32(Layer_attr(layer, Token_join(layerId, ".src"))),
               Attr_uint32(Layer_attr(layer, Token_join(layerId, ".dst"))),
               std::string(parentSrc.begin, Slice_length(parentSrc)),
               std::string(parentDst.begin, Slice_length(parentDst)));

  Stream &stream = worker->idMap[id];
  if (stream.id == 0) {
    stream.id = (worker->idMap.size() << 8) | Layer_worker(layer);
  }
  Attr_setUint32(Layer_addAttr(ctx, layer, streamIdToken), stream.id);
  const Slice payload =
      Payload_slices(Layer_payloads(layer, nullptr)[0], nullptr)[0];

  uint32_t seq = Attr_uint32(Layer_attr(layer, seqToken));
  uint16_t window = Attr_uint32(Layer_attr(layer, windowToken));
  uint8_t flags = Attr_uint32(Layer_attr(layer, flagsToken));
  bool syn = (flags & (0x1 << 1));
  if (syn) {
    if (stream.currentSeq < 0) {
      stream.currentSeq = seq;
      stream.ring.put(stream.receivedLength, payload);
      stream.receivedLength += Slice_length(payload);
    }
  } else if (stream.currentSeq >= 0) {
    if (Slice_length(payload)) {
      uint64_t start = stream.receivedLength;
      if (seq >= stream.currentSeq) {
        start += seq - stream.currentSeq;
        stream.currentSeq = seq;
        stream.ring.put(stream.receivedLength, payload);
        stream.receivedLength += Slice_length(payload);
      } else if (stream.currentSeq - seq > window) {
        start += (UINT32_MAX - stream.currentSeq) + seq;
        stream.currentSeq = seq;
        stream.ring.put(stream.receivedLength, payload);
        stream.receivedLength += Slice_length(payload);
      }
    } else if ((stream.currentSeq + 1) % UINT32_MAX == seq) {
      stream.currentSeq = seq;
    }
  }

  const auto slices = stream.ring.fetch();
  if (slices.size() > 0) {
    Payload *chunk = Layer_addPayload(ctx, layer);
    Payload_setType(chunk, streamToken);
    for (const auto &slice : slices) {
      Payload_addSlice(chunk, slice);
    }
  }

  Layer *sub = Layer_addSubLayer(ctx, layer, tcpStreamToken);
  Layer_addTag(sub, tcpStreamToken);
}
} // namespace

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("tcp");
  diss.analyze = analyze;
  diss.createWorker = [](Context *ctx, const Dissector *diss) {
    return Worker{new TCPWorker()};
  };
  diss.destroyWorker = [](Context *ctx, const Dissector *diss, Worker data) {
    TCPWorker *worker = static_cast<TCPWorker *>(data.data);
    delete worker;
  };
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
