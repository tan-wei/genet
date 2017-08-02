#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/dissection_result.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>

#include <plugkit/variant.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

namespace {

void analyze(Context *ctx, Worker *data, Layer *layer) {
  fmt::Reader<Slice> reader(layer->payload());
  Layer *child = Layer_addLayer(layer, Token_get("ipv4"));
  Layer_addTag(child, Token_get("ipv4"));

  uint8_t header = reader.readBE<uint8_t>();
  int version = header >> 4;
  int headerLength = header & 0b00001111;

  Property *ver = Layer_addProperty(child, Token_get("version"));
  *Property_valueRef(ver) = version;
  Property_setRange(ver, reader.lastRange());

  Property *hlen = Layer_addProperty(child, Token_get("hLen"));
  *Property_valueRef(hlen) = headerLength;
  Property_setRange(hlen, reader.lastRange());

  Property *tos = Layer_addProperty(child, Token_get("type"));
  *Property_valueRef(tos) = reader.readBE<uint8_t>();
  Property_setRange(tos, reader.lastRange());

  uint16_t totalLength = reader.readBE<uint16_t>();
  Property *tlen = Layer_addProperty(child, Token_get("tLen"));
  *Property_valueRef(tlen) = totalLength;
  Property_setRange(tlen, reader.lastRange());

  Property *id = Layer_addProperty(child, Token_get("id"));
  *Property_valueRef(id) = reader.readBE<uint16_t>();
  Property_setRange(id, reader.lastRange());

  uint8_t flagAndOffset = reader.readBE<uint8_t>();
  uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

  const std::tuple<uint16_t, const char *, Token> flagTable[] = {
      std::make_tuple(0x1, "Reserved", Token_get("reserved")),
      std::make_tuple(0x2, "Don\'t Fragment", Token_get("dontFrag")),
      std::make_tuple(0x4, "More Fragments", Token_get("moreFrag")),
  };

  Property *flags = Layer_addProperty(child, Token_get("flags"));
  *Property_valueRef(flags) = flag;
  std::string flagSummary;
  for (const auto &bit : flagTable) {
    bool on = std::get<0>(bit) & flag;
    Property *flagBit = Property_addProperty(flags, std::get<2>(bit));
    *Property_valueRef(flagBit) = on;
    Property_setRange(flagBit, reader.lastRange());

    if (on) {
      if (!flagSummary.empty())
        flagSummary += ", ";
      flagSummary += std::get<1>(bit);
    }
  }
  Property_setRange(flags, reader.lastRange());

  uint16_t fgOffset =
      ((flagAndOffset & 0b00011111) << 8) | reader.readBE<uint8_t>();
  Property *fragmentOffset = Layer_addProperty(child, Token_get("fOffset"));
  *Property_valueRef(fragmentOffset) = fgOffset;
  Property_setRange(fragmentOffset, Range{6, 8});

  Property *ttl = Layer_addProperty(child, Token_get("ttl"));
  *Property_valueRef(ttl) = reader.readBE<uint8_t>();
  Property_setRange(ttl, reader.lastRange());

  const std::unordered_map<uint16_t, std::pair<std::string, Token>> protoTable =
      {
          {0x01, std::make_pair("ICMP", Token_get("[icmp]"))},
          {0x02, std::make_pair("IGMP", Token_get("[igmp]"))},
          {0x06, std::make_pair("TCP", Token_get("[tcp]"))},
          {0x11, std::make_pair("UDP", Token_get("[udp]"))},
      };

  uint8_t protocolNumber = reader.readBE<uint8_t>();
  Property *proto = Layer_addProperty(child, Token_get("protocol"));
  *Property_valueRef(proto) = protocolNumber;
  const auto &type =
      fmt::enums(protoTable, protocolNumber,
                 std::make_pair("Unknown", Token_get("[unknown]")));

  Property_setRange(proto, reader.lastRange());

  Layer_addTag(child, type.second);

  Property *checksum = Layer_addProperty(child, Token_get("checksum"));
  *Property_valueRef(checksum) = reader.readBE<uint16_t>();
  Property_setRange(checksum, reader.lastRange());

  const auto &srcSlice = reader.slice(4);
  Property *src = Layer_addProperty(child, Token_get("src"));
  *Property_valueRef(src) = srcSlice;
  Property_setRange(src, reader.lastRange());

  const auto &dstSlice = reader.slice(4);
  Property *dst = Layer_addProperty(child, Token_get("dst"));
  *Property_valueRef(dst) = dstSlice;
  Property_setRange(dst, reader.lastRange());

  child->setPayload(reader.slice(totalLength - 20));
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
