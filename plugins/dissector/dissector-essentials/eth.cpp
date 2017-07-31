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
  Layer *child = new Layer(Token_get("eth"));
  child->addTag(Token_get("eth"));

  const auto &srcSlice = reader.slice(6);
  Property *src = new Property(Token_get("src"), srcSlice);
  src->setRange(reader.lastRange());

  child->addProperty(src);

  const auto &dstSlice = reader.slice(6);
  Property *dst = new Property(Token_get("dst"), dstSlice);

  dst->setRange(reader.lastRange());
  child->addProperty(dst);

  auto protocolType = reader.readBE<uint16_t>();
  if (protocolType <= 1500) {
    Property *length = new Property(Token_get("len"), protocolType);
    length->setRange(reader.lastRange());

    child->addProperty(length);
  } else {
    const std::unordered_map<uint16_t, std::pair<std::string, Token>>
        typeTable = {
            {0x0800, std::make_pair("IPv4", Token_get("[ipv4]"))},
            {0x86DD, std::make_pair("IPv6", Token_get("[ipv6]"))},
        };

    Property *etherType = new Property(Token_get("ethType"), protocolType);
    const auto &type =
        fmt::enums(typeTable, protocolType,
                   std::make_pair("Unknown", Token_get("[unknown]")));

    etherType->setRange(reader.lastRange());
    child->addProperty(etherType);
    child->addTag(type.second);
  }

  child->setPayload(reader.slice());
  result->child = child;
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
