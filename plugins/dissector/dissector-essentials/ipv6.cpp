#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>

#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

namespace {
int maxZeroSequence(const Slice &data) {
  int start = -1;
  int pos = -1;
  int length = 0;
  int maxLength = 0;
  for (size_t i = 0; i < data.length() / 2; ++i) {
    if (data[i * 2] == 0 && data[i * 2 + 1] == 0) {
      if (pos < 0) {
        pos = i;
      }
      ++length;
    } else {
      if (length > maxLength) {
        maxLength = length;
        start = pos;
      }
      pos = -1;
      length = 0;
    }
  }
  if (length > maxLength) {
    maxLength = length;
    start = pos;
  }
  return start;
}

std::string ipv6Addr(const Slice &data) {
  int zeroPos = maxZeroSequence(data);
  std::stringstream stream;
  stream << std::hex;
  int length = data.length() / 2;
  for (int i = 0; i < length; ++i) {
    if (zeroPos >= 0 && i >= zeroPos) {
      if (data[i * 2] == 0 && data[i * 2 + 1] == 0) {
        continue;
      } else {
        zeroPos = -1;
        stream << "::";
      }
    } else if (i > 0) {
      stream << ':';
    }
    stream << (static_cast<uint16_t>(data[i * 2] << 8) |
               static_cast<uint8_t>(data[i * 2 + 1]));
  }
  if (zeroPos >= 0) {
    stream << "::";
  }
  return stream.str();
}

const std::unordered_map<uint16_t, std::pair<std::string, Token>> protoTable = {
    {0x01, std::make_pair("ICMP", Token_get("[icmp]"))},
    {0x02, std::make_pair("IGMP", Token_get("[igmp]"))},
    {0x06, std::make_pair("TCP", Token_get("[tcp]"))},
    {0x11, std::make_pair("UDP", Token_get("[udp]"))},
};

const auto ipv6Token = Token_get("ipv6");
const auto versionToken = Token_get("version");
const auto tClassToken = Token_get("tClass");
const auto fLevelToken = Token_get("fLevel");
const auto pLenToken = Token_get("pLen");
const auto nHeaderToken = Token_get("nHeader");
const auto hLimitToken = Token_get("hLimit");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");
const auto hbyhToken = Token_get("hbyh");
const auto protocolToken = Token_get("protocol");
const auto unknownToken = Token_get("[unknown]");

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.view = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, ipv6Token);
  Layer_addTag(child, ipv6Token);

  uint8_t header = Reader_readUint8(&reader);
  uint8_t header2 = Reader_readUint8(&reader);
  int version = header >> 4;
  int trafficClass = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
  int flowLevel = Reader_readUint16BE(&reader) | ((header2 & 0b00001111) << 16);

  Property *ver = Layer_addProperty(child, versionToken);
  Variant_setUint64(Property_valueRef(ver), version);
  Property_setRange(ver, Range{0, 1});

  Property *tClass = Layer_addProperty(child, tClassToken);
  Variant_setUint64(Property_valueRef(tClass), trafficClass);
  Property_setRange(tClass, Range{0, 2});

  Property *fLevel = Layer_addProperty(child, fLevelToken);
  Variant_setUint64(Property_valueRef(fLevel), flowLevel);
  Property_setRange(fLevel, Range{1, 4});

  Property *pLen = Layer_addProperty(child, pLenToken);
  Variant_setUint64(Property_valueRef(pLen), Reader_readUint16BE(&reader));
  Property_setRange(pLen, reader.lastRange);

  int nextHeader = Reader_readUint8(&reader);
  auto nextHeaderRange = reader.lastRange;

  Property *nHeader = Layer_addProperty(child, nHeaderToken);
  Variant_setUint64(Property_valueRef(nHeader), nextHeader);
  Property_setRange(nHeader, nextHeaderRange);

  Property *hLimit = Layer_addProperty(child, hLimitToken);
  Variant_setUint64(Property_valueRef(hLimit), Reader_readUint8(&reader));
  Property_setRange(hLimit, reader.lastRange);

  const auto &srcSlice = Reader_slice(&reader, 0, 16);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setData(Property_valueRef(src), srcSlice);
  //       src->setSummary(ipv6Addr(srcSlice));
  Property_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 16);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setData(Property_valueRef(dst), dstSlice);
  //       dst->setSummary(ipv6Addr(dstSlice));
  Property_setRange(dst, reader.lastRange);

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
  Property *proto = Layer_addProperty(child, protocolToken);
  Variant_setUint64(Property_valueRef(proto), protocolNumber);
  const auto &type = fmt::enums(protoTable, protocolNumber,
                                std::make_pair("Unknown", unknownToken));
  //       proto->setSummary(type.first);
  Property_setRange(proto, reader.lastRange);
  Layer_addTag(child, type.second);

  /*
        const std::string &summary =
            (src->summary() > dst->summary())
                ? src->summary() + " -> " + dst->summary()
                : dst->summary() + " <- " + src->summary();

        child->setSummary("[" + proto->summary() + "] " + summary);
        */
  Layer_addPayload(child, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[ipv6]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
