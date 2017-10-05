#include <nan.h>
#include <plugkit/attribute.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <plugkit/token.h>
#include <plugkit/variant.h>
#include <unordered_map>

using namespace plugkit;

const auto udpToken = Token_get("udp");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");
const auto lengthToken = Token_get("udp.length");
const auto checksumToken = Token_get("udp.checksum");

namespace {
void analyze(Context *ctx, void *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.data = Payload_slices(Layer_payloads(layer, nullptr)[0], nullptr)[0];

  Layer *child = Layer_addLayer(layer, udpToken);
  Layer_addTag(child, udpToken);

  const auto &parentSrc = Layer_attr(layer, srcToken);
  const auto &parentDst = Layer_attr(layer, dstToken);

  uint16_t sourcePort = Reader_getUint16(&reader, false);
  Attr *src = Layer_addAttr(child, srcToken);
  Attr_setUint32(src, sourcePort);
  Attr_setRange(src, reader.lastRange);

  uint16_t dstPort = Reader_getUint16(&reader, false);
  Attr *dst = Layer_addAttr(child, dstToken);
  Attr_setUint32(dst, dstPort);
  Attr_setRange(dst, reader.lastRange);

  uint32_t lengthNumber = Reader_getUint16(&reader, false);
  Attr *length = Layer_addAttr(child, lengthToken);
  Attr_setUint32(length, lengthNumber);
  Attr_setRange(length, reader.lastRange);

  uint32_t checksumNumber = Reader_getUint16(&reader, false);
  Attr *checksum = Layer_addAttr(child, checksumToken);
  Attr_setUint32(checksum, checksumNumber);
  Attr_setRange(checksum, reader.lastRange);

  Payload *chunk = Layer_addPayload(child);
  Payload_addSlice(chunk, Reader_slice(&reader, 0, lengthNumber - 8));
}
} // namespace

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create();
  Dissector_addLayerHint(diss, Token_get("[udp]"));
  Dissector_setAnalyzer(diss, analyze);
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
