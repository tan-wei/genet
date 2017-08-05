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

const auto tcpToken = Token_get("tcp");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");
const auto seqToken = Token_get("seq");
const auto ackToken = Token_get("ack");
const auto dOffsetToken = Token_get("dOffset");
const auto flagsToken = Token_get("flags");
const auto windowToken = Token_get("window");
const auto checksumToken = Token_get("checksum");
const auto urgentToken = Token_get("urgent");
const auto optionsToken = Token_get("options");
const auto nopToken = Token_get("nop");
const auto mssToken = Token_get("mss");
const auto scaleToken = Token_get("scale");
const auto ackPermToken = Token_get("ackPerm");
const auto selAckToken = Token_get("selAck");
const auto tsToken = Token_get("ts");
const auto mtToken = Token_get("mt");
const auto etToken = Token_get("et");

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.view = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, tcpToken);
  Layer_addTag(child, tcpToken);

  const auto &parentSrc = Layer_propertyFromId(layer, srcToken);
  const auto &parentDst = Layer_propertyFromId(layer, dstToken);

  uint16_t sourcePort = Reader_readUint16BE(&reader);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setUint64(Property_valueRef(src), sourcePort);
  //       src->setSummary(parentSrc->summary() + ":" +
  //       std::to_string(sourcePort));
  Property_setRange(src, reader.lastRange);

  uint16_t dstPort = Reader_readUint16BE(&reader);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setUint64(Property_valueRef(dst), dstPort);
  //       dst->setSummary(parentDst->summary() + ":" +
  //       std::to_string(dstPort));
  Property_setRange(dst, reader.lastRange);

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

  uint32_t seqNumber = Reader_readUint32BE(&reader);
  Property *seq = Layer_addProperty(child, seqToken);
  Variant_setUint64(Property_valueRef(seq), seqNumber);
  Property_setRange(seq, reader.lastRange);

  uint32_t ackNumber = Reader_readUint32BE(&reader);
  Property *ack = Layer_addProperty(child, ackToken);
  Variant_setUint64(Property_valueRef(ack), ackNumber);
  Property_setRange(ack, reader.lastRange);

  uint8_t offsetAndFlag = Reader_readUint8(&reader);
  int dataOffset = offsetAndFlag >> 4;
  Property *offset = Layer_addProperty(child, dOffsetToken);
  Variant_setUint64(Property_valueRef(offset), dataOffset);
  Property_setRange(offset, reader.lastRange);

  uint8_t flag = Reader_readUint8(&reader) | ((offsetAndFlag & 0x1) << 8);

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
  //       flags->setSummary(flagSummary);
  Property_setRange(flags, Range{12, 14});

  Property *window = Layer_addProperty(child, windowToken);
  Variant_setUint64(Property_valueRef(window), Reader_readUint16BE(&reader));
  Property_setRange(window, reader.lastRange);

  Property *checksum = Layer_addProperty(child, checksumToken);
  Variant_setUint64(Property_valueRef(checksum), Reader_readUint16BE(&reader));
  Property_setRange(checksum, reader.lastRange);

  Property *urgent = Layer_addProperty(child, urgentToken);
  Variant_setUint64(Property_valueRef(urgent), Reader_readUint16BE(&reader));
  Property_setRange(urgent, reader.lastRange);

  Property *options = Layer_addProperty(child, optionsToken);
  Property_setRange(options, reader.lastRange);

  size_t optionDataOffset = dataOffset * 4;
  uint32_t optionOffset = 20;
  while (optionDataOffset > optionOffset) {
    switch (reader.view.begin[optionOffset]) {
    case 0:
      optionOffset = optionDataOffset;
      break;
    case 1: {
      Property *opt = Property_addProperty(options, nopToken);
      Property_setRange(opt, Range{optionOffset, optionOffset + 1});
      optionOffset++;
    } break;
    case 2: {
      uint16_t size = View_readUint16BE(reader.view, optionOffset + 2, nullptr);
      Property *opt = Property_addProperty(options, mssToken);
      Variant_setUint64(Property_valueRef(opt), size);
      Property_setRange(opt, Range{optionOffset, optionOffset + 4});
      optionOffset += 4;
    } break;
    case 3: {
      uint8_t scale = View_readUint8(reader.view, optionOffset + 2, nullptr);
      Property *opt = Property_addProperty(options, scaleToken);
      Variant_setUint64(Property_valueRef(opt), scale);
      Property_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 3;
    } break;
    case 4: {
      Property *opt = Property_addProperty(options, ackPermToken);
      Variant_setBool(Property_valueRef(opt), true);
      Property_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 2;
    } break;

    // TODO: https://tools.ietf.org/html/rfc2018
    case 5: {
      uint8_t length = View_readUint8(reader.view, optionOffset + 1, nullptr);
      Property *opt = Property_addProperty(options, selAckToken);
      Variant_setData(Property_valueRef(opt),
                      View_slice(reader.view, optionOffset + 2, length));
      Property_setRange(opt, Range{optionOffset, optionOffset + length});
      optionOffset += length;
    } break;
    case 8: {
      uint32_t mt = View_readUint32BE(reader.view, optionOffset + 2, nullptr);
      uint32_t et = View_readUint32BE(
          reader.view, optionOffset + 2 + sizeof(uint32_t), nullptr);
      Property *opt = Property_addProperty(options, tsToken);
      Variant_setString(
          Property_valueRef(opt),
          (std::to_string(mt) + " - " + std::to_string(et)).c_str());
      Property_setRange(opt, Range{optionOffset, optionOffset + 10});

      Property *optmt = Property_addProperty(opt, mtToken);
      Variant_setUint64(Property_valueRef(optmt), mt);
      Property_setRange(optmt, Range{optionOffset + 2, optionOffset + 6});

      Property *optet = Property_addProperty(opt, etToken);
      Variant_setUint64(Property_valueRef(optet), et);
      Property_setRange(optet, Range{optionOffset + 6, optionOffset + 10});

      optionOffset += 10;
    } break;
    default:

      optionOffset = optionDataOffset;
      break;
    }
  }

  Layer_addPayload(child, View_sliceAll(reader.view, optionDataOffset));
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
