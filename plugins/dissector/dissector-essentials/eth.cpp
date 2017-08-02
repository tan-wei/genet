#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/dissection_result.h>
#include <plugkit/context.h>
#include <plugkit/property.h>
#include <plugkit/token.h>

#include <plugkit/variant.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

namespace {

const auto ethToken = Token_get("eth");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");
const auto lenToken = Token_get("len");
const auto ethTypeToken = Token_get("ethType");
const auto unknownToken = Token_get("[unknown]");

static const std::unordered_map<uint16_t, std::pair<std::string, Token>>
    typeTable = {
        {0x0800, std::make_pair("IPv4", Token_get("[ipv4]"))},
        {0x86DD, std::make_pair("IPv6", Token_get("[ipv6]"))},
};

void analyze(Context *ctx, Worker *data, Layer *layer) {
  fmt::Reader<Slice> reader(layer->payload());
  Layer *child = Layer_addLayer(layer, ethToken);
  Layer_addTag(child, ethToken);

  const auto &srcSlice = reader.slice(6);
  Property *src = Layer_addProperty(child, srcToken);
  *Property_valueRef(src) = srcSlice;
  Property_setRange(src, reader.lastRange());

  const auto &dstSlice = reader.slice(6);
  Property *dst = Layer_addProperty(child, dstToken);
  *Property_valueRef(dst) = dstSlice;
  Property_setRange(dst, reader.lastRange());

  auto protocolType = reader.readBE<uint16_t>();
  if (protocolType <= 1500) {
    Property *length = Layer_addProperty(child, lenToken);
    *Property_valueRef(length) = protocolType;
    Property_setRange(length, reader.lastRange());
  } else {

    Property *etherType = Layer_addProperty(child, ethTypeToken);
    const auto &type = fmt::enums(typeTable, protocolType,
                                  std::make_pair("Unknown", unknownToken));

    *Property_valueRef(etherType) = protocolType;
    Property_setRange(etherType, reader.lastRange());
    Layer_addTag(child, type.second);
    ;
  }

  child->setPayload(reader.slice());
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
