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

const std::pair<uint16_t, Token> flagTable[] = {
    std::make_pair(0x1 << 8, Token_get("tcp.flags.ns")),
    std::make_pair(0x1 << 7, Token_get("tcp.flags.cwr")),
    std::make_pair(0x1 << 6, Token_get("tcp.flags.ece")),
    std::make_pair(0x1 << 5, Token_get("tcp.flags.urg")),
    std::make_pair(0x1 << 4, Token_get("tcp.flags.ack")),
    std::make_pair(0x1 << 3, Token_get("tcp.flags.psh")),
    std::make_pair(0x1 << 2, Token_get("tcp.flags.rst")),
    std::make_pair(0x1 << 1, Token_get("tcp.flags.syn")),
    std::make_pair(0x1 << 0, Token_get("tcp.flags.fin")),
};

const auto tcpToken = Token_get("tcp");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");
const auto seqToken = Token_get("tcp.seq");
const auto ackToken = Token_get("tcp.ack");
const auto dOffsetToken = Token_get("tcp.dOffset");
const auto flagsToken = Token_get("tcp.flags");
const auto windowToken = Token_get("tcp.window");
const auto checksumToken = Token_get("tcp.checksum");
const auto urgentToken = Token_get("tcp.urgent");
const auto optionsToken = Token_get("tcp.options");
const auto nopToken = Token_get("tcp.options.nop");
const auto mssToken = Token_get("tcp.options.mss");
const auto scaleToken = Token_get("tcp.options.scale");
const auto ackPermToken = Token_get("tcp.options.ackPerm");
const auto selAckToken = Token_get("tcp.options.selAck");
const auto tsToken = Token_get("tcp.options.ts");
const auto mtToken = Token_get("tcp.options.ts.mt");
const auto etToken = Token_get("tcp.options.ts.et");
const auto flagsTypeToken = Token_get("@flags");
const auto nestedToken = Token_get("@nested");

void analyze(Context *ctx, void *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_slice(Layer_payload(layer));

  Layer *child = Layer_addLayer(layer, tcpToken);
  Layer_addTag(child, tcpToken);

  const auto &parentSrc = Attr_slice(Layer_attr(layer, srcToken));
  const auto &parentDst = Attr_slice(Layer_attr(layer, dstToken));

  uint16_t srcPort = Reader_readUint16BE(&reader);
  Attr *src = Layer_addAttr(child, srcToken);
  Attr_setUint32(src, srcPort);
  Attr_setRange(src, reader.lastRange);

  uint16_t dstPort = Reader_readUint16BE(&reader);
  Attr *dst = Layer_addAttr(child, dstToken);
  Attr_setUint32(dst, dstPort);
  Attr_setRange(dst, reader.lastRange);

  Context_addStreamIdentifier(
      ctx, child, reinterpret_cast<const char *>(&srcPort), sizeof(uint16_t));
  Context_addStreamIdentifier(
      ctx, child, reinterpret_cast<const char *>(&dstPort), sizeof(uint16_t));
  Context_addStreamIdentifier(ctx, child, parentSrc.begin,
                              Slice_length(parentSrc));
  Context_addStreamIdentifier(ctx, child, parentDst.begin,
                              Slice_length(parentDst));

  uint32_t seqNumber = Reader_readUint32BE(&reader);
  Attr *seq = Layer_addAttr(child, seqToken);
  Attr_setUint32(seq, seqNumber);
  Attr_setRange(seq, reader.lastRange);

  uint32_t ackNumber = Reader_readUint32BE(&reader);
  Attr *ack = Layer_addAttr(child, ackToken);
  Attr_setUint32(ack, ackNumber);
  Attr_setRange(ack, reader.lastRange);

  uint8_t offsetAndFlag = Reader_readUint8(&reader);
  int dataOffset = offsetAndFlag >> 4;
  Attr *offset = Layer_addAttr(child, dOffsetToken);
  Attr_setUint32(offset, dataOffset);
  Attr_setRange(offset, reader.lastRange);

  uint8_t flag = Reader_readUint8(&reader) | ((offsetAndFlag & 0x1) << 8);

  Attr *flags = Layer_addAttr(child, flagsToken);
  Attr_setUint32(flags, flag);
  for (const auto &bit : flagTable) {
    bool on = bit.first & flag;
    Attr *flagBit = Layer_addAttr(child, bit.second);
    Attr_setBool(flagBit, on);
    Attr_setRange(flagBit, reader.lastRange);
  }
  Attr_setType(flags, flagsTypeToken);
  Attr_setRange(flags, Range{12, 14});

  Attr *window = Layer_addAttr(child, windowToken);
  Attr_setUint32(window, Reader_readUint16BE(&reader));
  Attr_setRange(window, reader.lastRange);

  Attr *checksum = Layer_addAttr(child, checksumToken);
  Attr_setUint32(checksum, Reader_readUint16BE(&reader));
  Attr_setRange(checksum, reader.lastRange);

  Attr *urgent = Layer_addAttr(child, urgentToken);
  Attr_setUint32(urgent, Reader_readUint16BE(&reader));
  Attr_setRange(urgent, reader.lastRange);

  Attr *options = Layer_addAttr(child, optionsToken);
  Attr_setType(options, nestedToken);
  Attr_setRange(options, reader.lastRange);

  size_t optionDataOffset = dataOffset * 4;
  uint32_t optionOffset = 20;
  while (optionDataOffset > optionOffset) {
    switch (reader.slice.begin[optionOffset]) {
    case 0:
      optionOffset = optionDataOffset;
      break;
    case 1: {
      Attr *opt = Layer_addAttr(child, nopToken);
      Attr_setRange(opt, Range{optionOffset, optionOffset + 1});
      optionOffset++;
    } break;
    case 2: {
      uint16_t size =
          Slice_readUint16BE(reader.slice, optionOffset + 2, nullptr);
      Attr *opt = Layer_addAttr(child, mssToken);
      Attr_setUint32(opt, size);
      Attr_setRange(opt, Range{optionOffset, optionOffset + 4});
      optionOffset += 4;
    } break;
    case 3: {
      uint8_t scale = Slice_readUint8(reader.slice, optionOffset + 2, nullptr);
      Attr *opt = Layer_addAttr(child, scaleToken);
      Attr_setUint32(opt, scale);
      Attr_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 3;
    } break;
    case 4: {
      Attr *opt = Layer_addAttr(child, ackPermToken);
      Attr_setRange(opt, Range{optionOffset, optionOffset + 2});
      optionOffset += 2;
    } break;

    // TODO: https://tools.ietf.org/html/rfc2018
    case 5: {
      uint8_t length = Slice_readUint8(reader.slice, optionOffset + 1, nullptr);
      Attr *opt = Layer_addAttr(child, selAckToken);
      Attr_setSlice(opt, Slice_slice(reader.slice, optionOffset + 2, length));
      Attr_setRange(opt, Range{optionOffset, optionOffset + length});
      optionOffset += length;
    } break;
    case 8: {
      uint32_t mt = Slice_readUint32BE(reader.slice, optionOffset + 2, nullptr);
      uint32_t et = Slice_readUint32BE(
          reader.slice, optionOffset + 2 + sizeof(uint32_t), nullptr);
      Attr *opt = Layer_addAttr(child, tsToken);
      Attr_setString(opt,
                     (std::to_string(mt) + " - " + std::to_string(et)).c_str());
      Attr_setRange(opt, Range{optionOffset, optionOffset + 10});

      Attr *optmt = Layer_addAttr(child, mtToken);
      Attr_setUint32(optmt, mt);
      Attr_setRange(optmt, Range{optionOffset + 2, optionOffset + 6});

      Attr *optet = Layer_addAttr(child, etToken);
      Attr_setUint32(optet, et);
      Attr_setRange(optet, Range{optionOffset + 6, optionOffset + 10});

      optionOffset += 10;
    } break;
    default:

      optionOffset = optionDataOffset;
      break;
    }
  }

  Payload *chunk = Layer_addPayload(child);
  Payload_addSlice(chunk, Slice_sliceAll(reader.slice, optionDataOffset));
}
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_PACKET);
  Dissector_addLayerHint(diss, Token_get("[tcp]"));
  Dissector_setAnalyzer(diss, analyze);
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
