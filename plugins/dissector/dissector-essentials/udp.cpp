#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>

#include <plugkit/variant.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

const auto udpToken = Token_get("udp");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");
const auto lengthToken = Token_get("length");
const auto checksumToken = Token_get("checksum");

namespace {
void analyze(Context *ctx, Worker *data, Layer *layer) {
  fmt::Reader<Slice> reader(layer->payload());
  Layer *child = Layer_addLayer(layer, udpToken);
  Layer_addTag(child, udpToken);
  ;

  const auto &parentSrc = layer->propertyFromId(srcToken);
  const auto &parentDst = layer->propertyFromId(dstToken);

  uint16_t sourcePort = reader.readBE<uint16_t>();
  Property *src = Layer_addProperty(child, srcToken);
  *Property_valueRef(src) = sourcePort;
  //       src->setSummary(parentSrc->summary() + ":" +
  //       std::to_string(sourcePort));
  Property_setRange(src, reader.lastRange());

  uint16_t dstPort = reader.readBE<uint16_t>();
  Property *dst = Layer_addProperty(child, dstToken);
  *Property_valueRef(dst) = dstPort;
  //       dst->setSummary(parentDst->summary() + ":" +
  //       std::to_string(dstPort));
  Property_setRange(dst, reader.lastRange());

  uint32_t lengthNumber = reader.readBE<uint16_t>();
  Property *length = Layer_addProperty(child, lengthToken);
  *Property_valueRef(length) = lengthNumber;
  Property_setRange(length, reader.lastRange());

  uint32_t checksumNumber = reader.readBE<uint16_t>();
  Property *checksum = Layer_addProperty(child, checksumToken);
  *Property_valueRef(checksum) = checksumNumber;
  Property_setRange(checksum, reader.lastRange());

  /*
        child->setSummary(src->summary() + " -> " + dst->summary());
        */
  child->setPayload(reader.slice(lengthNumber - 8));
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[udp]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
