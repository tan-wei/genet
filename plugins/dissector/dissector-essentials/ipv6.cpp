#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/dissection_result.h>
#include <plugkit/context.h>
#include <plugkit/token.h>

#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
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
}

namespace {
void analyze(Context *ctx, Worker *data, Layer *layer) {
  fmt::Reader<Slice> reader(layer->payload());
  Layer *child = Layer_addLayer(layer, Token_get("ipv6"));
  Layer_addTag(child, Token_get("ipv6"));
  ;

  uint8_t header = reader.readBE<uint8_t>();
  uint8_t header2 = reader.readBE<uint8_t>();
  int version = header >> 4;
  int trafficClass = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
  int flowLevel = reader.readBE<uint16_t>() | ((header2 & 0b00001111) << 16);

  Property *ver = Layer_addProperty(child, Token_get("version"));
  *Property_valueRef(ver) = version;
  Property_setRange(ver, Range{0, 1});

  Property *tClass = Layer_addProperty(child, Token_get("tClass"));
  *Property_valueRef(tClass) = trafficClass;
  Property_setRange(tClass, Range{0, 2});

  Property *fLevel = Layer_addProperty(child, Token_get("fLevel"));
  *Property_valueRef(fLevel) = flowLevel;
  Property_setRange(fLevel, Range{1, 4});

  Property *pLen = Layer_addProperty(child, Token_get("pLen"));
  *Property_valueRef(pLen) = reader.readBE<uint16_t>();
  Property_setRange(pLen, reader.lastRange());

  int nextHeader = reader.readBE<uint8_t>();
  auto nextHeaderRange = reader.lastRange();

  Property *nHeader = Layer_addProperty(child, Token_get("nHeader"));
  *Property_valueRef(nHeader) = nextHeader;
  Property_setRange(nHeader, nextHeaderRange);

  Property *hLimit =
      new Property(Token_get("hLimit"), reader.readBE<uint8_t>());
  Property_setRange(hLimit, reader.lastRange());

  const auto &srcSlice = reader.slice(16);
  Property *src = Layer_addProperty(child, Token_get("src"));
  *Property_valueRef(src) = srcSlice;
  //       src->setSummary(ipv6Addr(srcSlice));
  Property_setRange(src, reader.lastRange());

  const auto &dstSlice = reader.slice(16);
  Property *dst = Layer_addProperty(child, Token_get("dst"));
  *Property_valueRef(dst) = dstSlice;
  //       dst->setSummary(ipv6Addr(dstSlice));
  Property_setRange(dst, reader.lastRange());

  bool ext = true;
  while (ext) {
    Property *item = nullptr;
    int header = 0;
    switch (nextHeader) {
    case 0:
    case 60: // Hop-by-Hop Options, Destination Options
    {
      header = reader.readBE<uint8_t>();
      size_t extLen = reader.readBE<uint8_t>();
      size_t byteLen = (extLen + 1) * 8;
      reader.slice(byteLen);
      Token id = (nextHeader == 0) ? Token_get("hbyh") : Token_get("dst");
      item = new Property(id);
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

  const std::unordered_map<uint16_t, std::pair<std::string, Token>> protoTable =
      {
          {0x01, std::make_pair("ICMP", Token_get("[icmp]"))},
          {0x02, std::make_pair("IGMP", Token_get("[igmp]"))},
          {0x06, std::make_pair("TCP", Token_get("[tcp]"))},
          {0x11, std::make_pair("UDP", Token_get("[udp]"))},
      };

  uint8_t protocolNumber = nextHeader;
  Property *proto = Layer_addProperty(child, Token_get("protocol"));
  *Property_valueRef(proto) = protocolNumber;
  const auto &type =
      fmt::enums(protoTable, protocolNumber,
                 std::make_pair("Unknown", Token_get("[unknown]")));
  //       proto->setSummary(type.first);
  Property_setRange(proto, reader.lastRange());

  /*
        const std::string &summary =
            (src->summary() > dst->summary())
                ? src->summary() + " -> " + dst->summary()
                : dst->summary() + " <- " + src->summary();

        child->setSummary("[" + proto->summary() + "] " + summary);
        */
  child->setPayload(reader.slice());
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
