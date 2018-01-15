#include <plugkit/attr.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/module.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <plugkit/token.h>
#include <plugkit/variant.h>
#include <unordered_map>

using namespace plugkit;

const auto udpToken = Token_get("udp");
const auto srcToken = Token_get("udp.src");
const auto dstToken = Token_get("udp.dst");
const auto lengthToken = Token_get("udp.length");
const auto checksumToken = Token_get("udp.checksum");

namespace {
void analyze(Context *ctx, const Dissector *diss, Worker data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);

  const Payload *parentPayload = Layer_payloads(layer, nullptr)[0];
  Range payloadRange = Payload_range(parentPayload);
  reader.data = Payload_slices(parentPayload, nullptr)[0];

  Layer *child = Layer_addLayer(layer, ctx, udpToken);
  Layer_addTag(child, udpToken);
  Layer_setRange(child, payloadRange);

  uint16_t sourcePort = Reader_getUint16(&reader, false);
  Attr *src = Layer_addAttr(child, ctx, srcToken);
  Attr_setUint32(src, sourcePort);
  Attr_setRange(src, reader.lastRange);

  uint16_t dstPort = Reader_getUint16(&reader, false);
  Attr *dst = Layer_addAttr(child, ctx, dstToken);
  Attr_setUint32(dst, dstPort);
  Attr_setRange(dst, reader.lastRange);

  uint32_t lengthNumber = Reader_getUint16(&reader, false);
  Attr *length = Layer_addAttr(child, ctx, lengthToken);
  Attr_setUint32(length, lengthNumber);
  Attr_setRange(length, reader.lastRange);

  uint32_t checksumNumber = Reader_getUint16(&reader, false);
  Attr *checksum = Layer_addAttr(child, ctx, checksumToken);
  Attr_setUint32(checksum, checksumNumber);
  Attr_setRange(checksum, reader.lastRange);

  Payload *chunk = Layer_addPayload(child, ctx);
  Payload_addSlice(chunk, Reader_slice(&reader, 0, lengthNumber - 8));
  Payload_setRange(chunk, Range_offset(reader.lastRange, payloadRange.begin));
}
} // namespace

PLUGKIT_MODULE([]() {
  static Dissector diss;
  diss.layerHints[0] = (Token_get("[udp]"));
  diss.analyze = analyze;
  return &diss;
})
