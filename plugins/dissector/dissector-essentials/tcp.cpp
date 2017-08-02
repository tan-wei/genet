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
  Layer *child = Layer_addLayer(layer, Token_get("tcp"));
  Layer_addTag(child, Token_get("tcp"));
  ;

  const auto &parentSrc = layer->propertyFromId(Token_get("src"));
  const auto &parentDst = layer->propertyFromId(Token_get("dst"));

  uint16_t sourcePort = reader.readBE<uint16_t>();
  Property *src = Layer_addProperty(child, Token_get("src"));
  *Property_valueRef(src) = sourcePort;
  //       src->setSummary(parentSrc->summary() + ":" +
  //       std::to_string(sourcePort));
  Property_setRange(src, reader.lastRange());

  uint16_t dstPort = reader.readBE<uint16_t>();
  Property *dst = Layer_addProperty(child, Token_get("dst"));
  *Property_valueRef(dst) = dstPort;
  //       dst->setSummary(parentDst->summary() + ":" +
  //       std::to_string(dstPort));
  Property_setRange(dst, reader.lastRange());

  /*
        const std::string &summary =
            (src->summary() > dst->summary())
                ? src->summary() + " -> " + dst->summary()
                : dst->summary() + " <- " + src->summary();
        child->setSummary(summary);
  size_t size = summary.copy(meta->streamIdentifier,
                             sizeof(meta->streamIdentifier) - 1);

  for (size_t i = 0; i < size; ++i) {
    char &c = meta->streamIdentifier[i];
    if (c == '<' || c == '>' || c == '-')
      c = ' ';
  }
  meta->streamIdentifier[size] = '\0';
  */

  uint32_t seqNumber = reader.readBE<uint32_t>();
  Property *seq = Layer_addProperty(child, Token_get("seq"));
  *Property_valueRef(seq) = seqNumber;
  Property_setRange(seq, reader.lastRange());

  uint32_t ackNumber = reader.readBE<uint32_t>();
  Property *ack = Layer_addProperty(child, Token_get("ack"));
  *Property_valueRef(ack) = ackNumber;
  Property_setRange(ack, reader.lastRange());

  uint8_t offsetAndFlag = reader.readBE<uint8_t>();
  int dataOffset = offsetAndFlag >> 4;
  Property *offset = Layer_addProperty(child, Token_get("dOffset"));
  *Property_valueRef(offset) = dataOffset;
  Property_setRange(offset, reader.lastRange());

  uint8_t flag = reader.readBE<uint8_t>() | ((offsetAndFlag & 0x1) << 8);

  const std::tuple<uint16_t, const char *, Token> flagTable[] = {
      std::make_tuple(0x1 << 8, "NS", Token_get("ns")),
      std::make_tuple(0x1 << 7, "CWR", Token_get("cwr")),
      std::make_tuple(0x1 << 6, "ECE", Token_get("ece")),
      std::make_tuple(0x1 << 5, "URG", Token_get("urg")),
      std::make_tuple(0x1 << 4, "ACK", Token_get("ack")),
      std::make_tuple(0x1 << 3, "PSH", Token_get("psh")),
      std::make_tuple(0x1 << 2, "RST", Token_get("rst")),
      std::make_tuple(0x1 << 1, "SYN", Token_get("syn")),
      std::make_tuple(0x1 << 0, "FIN", Token_get("fin")),
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
  //       flags->setSummary(flagSummary);
  Property_setRange(flags, Range{12, 14});

  Property *window = Layer_addProperty(child, Token_get("window"));
  *Property_valueRef(window) = reader.readBE<uint16_t>();
  Property_setRange(window, reader.lastRange());

  Property *checksum = Layer_addProperty(child, Token_get("checksum"));
  *Property_valueRef(checksum) = reader.readBE<uint16_t>();
  Property_setRange(checksum, reader.lastRange());

  Property *urgent = Layer_addProperty(child, Token_get("urgent"));
  *Property_valueRef(urgent) = reader.readBE<uint16_t>();
  Property_setRange(urgent, reader.lastRange());

  Property *options = Layer_addProperty(child, Token_get("options"));
  Property_setRange(options, reader.lastRange());

  size_t optionDataOffset = dataOffset * 4;
  uint32_t optionOffset = 20;
  while (optionDataOffset > optionOffset) {
    switch (layer->payload()[optionOffset]) {
    case 0:
      optionOffset = optionDataOffset;
      break;
    case 1: {
      Property *opt = Property_addProperty(options, Token_get("nop"));
      Property_setRange(opt, Range{optionOffset, optionOffset + 1});
      optionOffset++;
    } break;
    case 2: {
      uint16_t size = fmt::readBE<uint16_t>(layer->payload(), optionOffset + 2);
      Property *opt = Property_addProperty(options, Token_get("mss"));
      *Property_valueRef(opt) = size;
      Property_setRange(opt, Range{optionOffset, optionOffset + 4});
      optionOffset += 4;
    } break;
    case 3: {
      uint8_t scale = fmt::readBE<uint8_t>(layer->payload(), optionOffset + 2);
      Property *opt = Property_addProperty(options, Token_get("scale"));
      *Property_valueRef(opt) = scale;
      Property_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 3;
    } break;

    case 4: {
      Property *opt = Property_addProperty(options, Token_get("ackPerm"));
      *Property_valueRef(opt) = true;
      Property_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 2;
    } break;

    // TODO: https://tools.ietf.org/html/rfc2018
    case 5: {
      uint8_t length = fmt::readBE<uint8_t>(layer->payload(), optionOffset + 1);
      Property *opt = Property_addProperty(options, Token_get("selAck"));
      *Property_valueRef(opt) =
          layer->payload().slice(optionOffset + 2, length);
      Property_setRange(opt, Range{optionOffset, optionOffset + length});
      optionOffset += length;
    } break;
    case 8: {
      uint32_t mt = fmt::readBE<uint32_t>(layer->payload(), optionOffset + 2);
      uint32_t et = fmt::readBE<uint32_t>(layer->payload(),
                                          optionOffset + 2 + sizeof(uint32_t));
      Property *opt = Property_addProperty(options, Token_get("ts"));
      *Property_valueRef(opt) = std::to_string(mt) + " - " + std::to_string(et);
      Property_setRange(opt, Range{optionOffset, optionOffset + 10});

      Property *optmt = Property_addProperty(opt, Token_get("mt"));
      *Property_valueRef(optmt) = mt;
      Property_setRange(optmt, Range{optionOffset + 2, optionOffset + 6});

      Property *optet = Property_addProperty(opt, Token_get("et"));
      *Property_valueRef(optet) = et;
      Property_setRange(optet, Range{optionOffset + 6, optionOffset + 10});

      optionOffset += 10;
    } break;
    default:

      optionOffset = optionDataOffset;
      break;
    }
  }

  const auto &payload = layer->payload().slice(optionDataOffset);
  child->setPayload(payload);
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[tcp]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
