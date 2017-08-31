#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/attribute.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <unordered_map>

using namespace plugkit;

namespace {

const std::unordered_map<uint16_t, std::pair<Token, Token>> protoTable = {
    {0x01,
     std::make_pair(Token_get("[icmp]"), Token_get("ipv6.protocol.icmp"))},
    {0x02,
     std::make_pair(Token_get("[igmp]"), Token_get("ipv6.protocol.igmp"))},
    {0x06, std::make_pair(Token_get("[tcp]"), Token_get("ipv6.protocol.tcp"))},
    {0x11, std::make_pair(Token_get("[udp]"), Token_get("ipv6.protocol.udp"))},
};

const auto ipv6Token = Token_get("ipv6");
const auto versionToken = Token_get("ipv6.version");
const auto tClassToken = Token_get("ipv6.tClass");
const auto fLevelToken = Token_get("ipv6.fLevel");
const auto pLenToken = Token_get("ipv6.pLen");
const auto nHeaderToken = Token_get("ipv6.nHeader");
const auto hLimitToken = Token_get("ipv6.hLimit");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");
const auto hbyhToken = Token_get("ipv6.hbyh");
const auto protocolToken = Token_get("ipv6.protocol");
const auto ipv6AddrToken = Token_get("@ipv6:addr");
const auto flagsTypeToken = Token_get("@flags");
const auto enumToken = Token_get("@enum");

void analyze(Context *ctx, void *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_slice(Layer_payload(layer));

  Layer *child = Layer_addLayer(layer, ipv6Token);
  Layer_addTag(child, ipv6Token);

  uint8_t header = Reader_readUint8(&reader);
  uint8_t header2 = Reader_readUint8(&reader);
  int version = header >> 4;
  int trafficClass = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
  int flowLevel = Reader_readUint16BE(&reader) | ((header2 & 0b00001111) << 16);

  Attr *ver = Layer_addAttr(child, versionToken);
  Attr_setUint32(ver, version);
  Attr_setRange(ver, Range{0, 1});

  Attr *tClass = Layer_addAttr(child, tClassToken);
  Attr_setUint32(tClass, trafficClass);
  Attr_setRange(tClass, Range{0, 2});

  Attr *fLevel = Layer_addAttr(child, fLevelToken);
  Attr_setUint32(fLevel, flowLevel);
  Attr_setRange(fLevel, Range{1, 4});

  Attr *pLen = Layer_addAttr(child, pLenToken);
  Attr_setUint32(pLen, Reader_readUint16BE(&reader));
  Attr_setRange(pLen, reader.lastRange);

  int nextHeader = Reader_readUint8(&reader);
  auto nextHeaderRange = reader.lastRange;

  Attr *nHeader = Layer_addAttr(child, nHeaderToken);
  Attr_setUint32(nHeader, nextHeader);
  Attr_setRange(nHeader, nextHeaderRange);

  Attr *hLimit = Layer_addAttr(child, hLimitToken);
  Attr_setUint32(hLimit, Reader_readUint8(&reader));
  Attr_setRange(hLimit, reader.lastRange);

  const auto &srcSlice = Reader_slice(&reader, 0, 16);
  Attr *src = Layer_addAttr(child, srcToken);
  Attr_setSlice(src, srcSlice);
  Attr_setType(src, ipv6AddrToken);
  Attr_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 16);
  Attr *dst = Layer_addAttr(child, dstToken);
  Attr_setSlice(dst, dstSlice);
  Attr_setType(dst, ipv6AddrToken);
  Attr_setRange(dst, reader.lastRange);

  bool ext = true;
  while (ext) {
    int header = 0;
    switch (nextHeader) {
    case 0:
    case 60: // Hop-by-Hop Options, Destination Options
    {
      header = Reader_readUint8(&reader);
      size_t extLen = Reader_readUint8(&reader);
      size_t byteLen = (extLen + 1) * 8;
      Reader_slice(&reader, 0, byteLen);
      Token id = (nextHeader == 0) ? hbyhToken : dstToken;
    }

    break;
    // TODO:
    // case 43  # Routing
    // case 44  # Fragment
    // case 51  # Authentication Header
    // case 50  # Encapsulating Security Payload
    // case 135 # Mobility
    case 59: // No Next Header
    default:
      ext = false;
      continue;
    }

    nextHeader = header;
  }

  uint8_t protocolNumber = nextHeader;
  Attr *proto = Layer_addAttr(child, protocolToken);
  Attr_setUint32(proto, protocolNumber);
  Attr_setType(proto, enumToken);
  Attr_setRange(proto, reader.lastRange);
  const auto &it = protoTable.find(protocolNumber);
  if (it != protoTable.end()) {
    Attr *sub = Layer_addAttr(child, it->second.second);
    Attr_setBool(sub, true);
    Attr_setRange(sub, reader.lastRange);
    Layer_addTag(child, it->second.first);
  }

  Payload *chunk = Layer_addPayload(child);
  Payload_addSlice(chunk, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_PACKET);
  Dissector_addLayerHint(diss, Token_get("[ipv6]"));
  Dissector_setAnalyzer(diss, analyze);
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
