#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <unordered_map>

using namespace plugkit;

namespace {

const std::tuple<uint16_t, const char *, Token> flagTable[] = {
    std::make_tuple(0x1, "Reserved", Token_get("reserved")),
    std::make_tuple(0x2, "Don\'t Fragment", Token_get("dontFrag")),
    std::make_tuple(0x4, "More Fragments", Token_get("moreFrag")),
};

const std::unordered_map<uint16_t, Token> protoTable = {
    {0x01, Token_get("[icmp]")},
    {0x02, Token_get("[igmp]")},
    {0x06, Token_get("[tcp]")},
    {0x11, Token_get("[udp]")},
};

const auto ipv4Token = Token_get("ipv4");
const auto versionToken = Token_get("version");
const auto hLenToken = Token_get("hLen");
const auto typeToken = Token_get("type");
const auto tLenToken = Token_get("tLen");
const auto idToken = Token_get("id");
const auto flagsToken = Token_get("flags");
const auto fOffsetToken = Token_get("fOffset");
const auto ttlToken = Token_get("ttl");
const auto protocolToken = Token_get("protocol");
const auto checksumToken = Token_get("checksum");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, ipv4Token);
  Layer_addTag(child, ipv4Token);

  uint8_t header = Reader_readUint8(&reader);
  int version = header >> 4;
  int headerLength = header & 0b00001111;

  Property *ver = Layer_addProperty(child, versionToken);
  Variant_setUint64(Property_valueRef(ver), version);
  Property_setRange(ver, reader.lastRange);

  Property *hlen = Layer_addProperty(child, hLenToken);
  Variant_setUint64(Property_valueRef(hlen), headerLength);
  Property_setRange(hlen, reader.lastRange);

  Property *tos = Layer_addProperty(child, typeToken);
  Variant_setUint64(Property_valueRef(tos), Reader_readUint8(&reader));
  Property_setRange(tos, reader.lastRange);

  uint16_t totalLength = Reader_readUint16BE(&reader);
  Property *tlen = Layer_addProperty(child, tLenToken);
  Variant_setUint64(Property_valueRef(tlen), totalLength);
  Property_setRange(tlen, reader.lastRange);

  Property *id = Layer_addProperty(child, idToken);
  Variant_setUint64(Property_valueRef(id), Reader_readUint16BE(&reader));
  Property_setRange(id, reader.lastRange);

  uint8_t flagAndOffset = Reader_readUint8(&reader);
  uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

  Property *flags = Layer_addProperty(child, flagsToken);
  Variant_setUint64(Property_valueRef(flags), flag);
  std::string flagSummary;
  for (const auto &bit : flagTable) {
    bool on = std::get<0>(bit) & flag;
    Property *flagBit = Property_addProperty(flags, std::get<2>(bit));
    Variant_setBool(Property_valueRef(flagBit), on);
    Property_setRange(flagBit, reader.lastRange);

    if (on) {
      if (!flagSummary.empty())
        flagSummary += ", ";
      flagSummary += std::get<1>(bit);
    }
  }
  Property_setRange(flags, reader.lastRange);

  uint16_t fgOffset =
      ((flagAndOffset & 0b00011111) << 8) | Reader_readUint8(&reader);
  Property *fragmentOffset = Layer_addProperty(child, fOffsetToken);
  Variant_setUint64(Property_valueRef(fragmentOffset), fgOffset);
  Property_setRange(fragmentOffset, Range{6, 8});

  Property *ttl = Layer_addProperty(child, ttlToken);
  Variant_setUint64(Property_valueRef(ttl), Reader_readUint8(&reader));
  Property_setRange(ttl, reader.lastRange);

  uint8_t protocolNumber = Reader_readUint8(&reader);
  Property *proto = Layer_addProperty(child, protocolToken);
  Variant_setUint64(Property_valueRef(proto), protocolNumber);
  Property_setRange(proto, reader.lastRange);
  const auto &it = protoTable.find(protocolNumber);
  if (it != protoTable.end()) {
    Layer_addTag(child, it->second);
  }

  Property *checksum = Layer_addProperty(child, checksumToken);
  Variant_setUint64(Property_valueRef(checksum), Reader_readUint16BE(&reader));
  Property_setRange(checksum, reader.lastRange);

  const auto &srcSlice = Reader_slice(&reader, 0, 4);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setData(Property_valueRef(src), srcSlice);
  Property_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 4);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setData(Property_valueRef(dst), dstSlice);
  Property_setRange(dst, reader.lastRange);

  Layer_addPayload(child, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[ipv4]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
