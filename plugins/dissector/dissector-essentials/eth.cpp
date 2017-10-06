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

namespace {

const auto ethToken = Token_get("eth");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");
const auto lenToken = Token_get("eth.len");
const auto ethTypeToken = Token_get("eth.type");
const auto macToken = Token_get("@eth:mac");

static const std::unordered_map<uint16_t, std::pair<Token, Token>> typeTable = {
    {0x0800, std::make_pair(Token_get("[ipv4]"), Token_get("eth.type.ipv4"))},
    {0x86DD, std::make_pair(Token_get("[ipv6]"), Token_get("eth.type.ipv6"))},
};

void analyze(Context *ctx, const Dissector *diss, Worker worker, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.data = Payload_slices(Layer_payloads(layer, nullptr)[0], nullptr)[0];

  Layer *child = Layer_addLayer(layer, ethToken);
  Layer_addTag(child, ethToken);

  const auto &srcSlice = Reader_slice(&reader, 0, 6);
  Attr *src = Layer_addAttr(child, srcToken);
  Attr_setSlice(src, srcSlice);
  Attr_setType(src, macToken);
  Attr_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 6);
  Attr *dst = Layer_addAttr(child, dstToken);
  Attr_setSlice(dst, dstSlice);
  Attr_setType(dst, macToken);
  Attr_setRange(dst, reader.lastRange);

  auto protocolType = Reader_getUint16(&reader, false);
  if (protocolType <= 1500) {
    Attr *length = Layer_addAttr(child, lenToken);
    Attr_setUint32(length, protocolType);
    Attr_setRange(length, reader.lastRange);
  } else {
    Attr *etherType = Layer_addAttr(child, ethTypeToken);
    Attr_setUint32(etherType, protocolType);
    Attr_setRange(etherType, reader.lastRange);
    const auto &it = typeTable.find(protocolType);
    if (it != typeTable.end()) {
      Attr *type = Layer_addAttr(child, it->second.second);
      Attr_setBool(type, true);
      Attr_setRange(type, reader.lastRange);
      Layer_addTag(child, it->second.first);
    }
  }

  Payload *chunk = Layer_addPayload(child);
  Payload_addSlice(chunk, Reader_sliceAll(&reader, 0));
}
} // namespace

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = (Token_get("[eth]"));
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
