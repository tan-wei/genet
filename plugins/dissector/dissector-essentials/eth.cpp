#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/property.h>
#include <plugkit/token.h>
#include <plugkit/reader.h>
#include <plugkit/variant.h>
#include <plugkit/payload.h>
#include <plugkit/layer.h>
#include <plugkit/logger.h>
#include <unordered_map>

using namespace plugkit;

namespace {

const auto ethToken = Token_get("eth");
const auto srcToken = Token_get("eth.src");
const auto dstToken = Token_get("eth.dst");
const auto lenToken = Token_get("eth.len");
const auto ethTypeToken = Token_get("eth.type");
const auto macToken = Token_get("@eth:mac");

static const std::unordered_map<uint16_t, std::pair<Token, Token>> typeTable = {
    {0x0800, std::make_pair(Token_get("[ipv4]"), Token_get("eth.type.ipv4"))},
    {0x86DD, std::make_pair(Token_get("[ipv6]"), Token_get("eth.type.ipv6"))},
};

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, ethToken);
  Layer_addTag(child, ethToken);

  const auto &srcSlice = Reader_slice(&reader, 0, 6);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setSlice(Property_valueRef(src), srcSlice);
  Property_setType(src, macToken);
  Property_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 6);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setSlice(Property_valueRef(dst), dstSlice);
  Property_setType(dst, macToken);
  Property_setRange(dst, reader.lastRange);

  auto protocolType = Reader_readUint16BE(&reader);
  if (protocolType <= 1500) {
    Property *length = Layer_addProperty(child, lenToken);
    Variant_setUint64(Property_valueRef(length), protocolType);
    Property_setRange(length, reader.lastRange);
  } else {
    Property *etherType = Layer_addProperty(child, ethTypeToken);
    Variant_setUint64(Property_valueRef(etherType), protocolType);
    Property_setRange(etherType, reader.lastRange);
    const auto &it = typeTable.find(protocolType);
    if (it != typeTable.end()) {
      Property *type = Layer_addProperty(child, it->second.second);
      Variant_setBool(Property_valueRef(type), true);
      Property_setRange(type, reader.lastRange);
      Layer_addTag(child, it->second.first);
    }
  }

  Layer_addPayload(child, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[eth]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
