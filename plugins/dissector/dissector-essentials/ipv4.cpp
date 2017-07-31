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

void analyze(Context *ctx, Worker *data, Layer *layer,
             DissectionResult *result) {
  fmt::Reader<Slice> reader(layer->payload());
  Layer *child = new Layer(Token_get("ipv4"));
  child->addTag(Token_get("ipv4"));

  uint8_t header = reader.readBE<uint8_t>();
  int version = header >> 4;
  int headerLength = header & 0b00001111;

  Property *ver = new Property(Token_get("version"), version);
  ver->setRange(reader.lastRange());

  child->addProperty(ver);

  Property *hlen = new Property(Token_get("hLen"), headerLength);
  hlen->setRange(reader.lastRange());

  child->addProperty(hlen);

  Property *tos = new Property(Token_get("type"), reader.readBE<uint8_t>());
  tos->setRange(reader.lastRange());

  child->addProperty(tos);

  uint16_t totalLength = reader.readBE<uint16_t>();
  Property *tlen = new Property(Token_get("tLen"), totalLength);
  tlen->setRange(reader.lastRange());

  child->addProperty(tlen);

  Property *id = new Property(Token_get("id"), reader.readBE<uint16_t>());
  id->setRange(reader.lastRange());

  child->addProperty(id);

  uint8_t flagAndOffset = reader.readBE<uint8_t>();
  uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

  const std::tuple<uint16_t, const char *, Token> flagTable[] = {
      std::make_tuple(0x1, "Reserved", Token_get("reserved")),
      std::make_tuple(0x2, "Don\'t Fragment", Token_get("dontFrag")),
      std::make_tuple(0x4, "More Fragments", Token_get("moreFrag")),
  };

  Property *flags = new Property(Token_get("flags"), flag);
  std::string flagSummary;
  for (const auto &bit : flagTable) {
    bool on = std::get<0>(bit) & flag;
    Property *flagBit = new Property(std::get<2>(bit), on);
    flagBit->setRange(reader.lastRange());

    flags->addProperty(flagBit);
    if (on) {
      if (!flagSummary.empty())
        flagSummary += ", ";
      flagSummary += std::get<1>(bit);
    }
  }
  flags->setRange(reader.lastRange());

  child->addProperty(flags);

  uint16_t fgOffset =
      ((flagAndOffset & 0b00011111) << 8) | reader.readBE<uint8_t>();
  Property *fragmentOffset = new Property(Token_get("fOffset"), fgOffset);
  fragmentOffset->setRange(std::make_pair(6, 8));

  child->addProperty(fragmentOffset);

  Property *ttl = new Property(Token_get("ttl"), reader.readBE<uint8_t>());
  ttl->setRange(reader.lastRange());

  child->addProperty(ttl);

  const std::unordered_map<uint16_t, std::pair<std::string, Token>> protoTable =
      {
          {0x01, std::make_pair("ICMP", Token_get("[icmp]"))},
          {0x02, std::make_pair("IGMP", Token_get("[igmp]"))},
          {0x06, std::make_pair("TCP", Token_get("[tcp]"))},
          {0x11, std::make_pair("UDP", Token_get("[udp]"))},
      };

  uint8_t protocolNumber = reader.readBE<uint8_t>();
  Property *proto = new Property(Token_get("protocol"), protocolNumber);
  const auto &type =
      fmt::enums(protoTable, protocolNumber,
                 std::make_pair("Unknown", Token_get("[unknown]")));

  proto->setRange(reader.lastRange());
  child->addProperty(proto);
  child->addTag(type.second);

  Property *checksum =
      new Property(Token_get("checksum"), reader.readBE<uint16_t>());
  checksum->setRange(reader.lastRange());

  child->addProperty(checksum);

  const auto &srcSlice = reader.slice(4);
  Property *src = new Property(Token_get("src"), srcSlice);
  src->setRange(reader.lastRange());

  child->addProperty(src);

  const auto &dstSlice = reader.slice(4);
  Property *dst = new Property(Token_get("dst"), dstSlice);
  dst->setRange(reader.lastRange());
  child->addProperty(dst);

  child->setPayload(reader.slice(totalLength - 20));
  result->child = child;
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
