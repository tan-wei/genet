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
  Layer *child = new Layer(Token_get("udp"));
  child->addTag(Token_get("udp"));

  const auto &parentSrc = layer->propertyFromId(Token_get("src"));
  const auto &parentDst = layer->propertyFromId(Token_get("dst"));

  uint16_t sourcePort = reader.readBE<uint16_t>();
  Property *src = new Property(Token_get("src"), sourcePort);
  //       src->setSummary(parentSrc->summary() + ":" +
  //       std::to_string(sourcePort));
  src->setRange(reader.lastRange());

  child->addProperty(src);

  uint16_t dstPort = reader.readBE<uint16_t>();
  Property *dst = new Property(Token_get("dst"), dstPort);
  //       dst->setSummary(parentDst->summary() + ":" +
  //       std::to_string(dstPort));
  dst->setRange(reader.lastRange());

  child->addProperty(dst);

  uint32_t lengthNumber = reader.readBE<uint16_t>();
  Property *length = new Property(Token_get("length"), lengthNumber);
  length->setRange(reader.lastRange());

  child->addProperty(length);

  uint32_t checksumNumber = reader.readBE<uint16_t>();
  Property *checksum = new Property(Token_get("checksum"), checksumNumber);
  checksum->setRange(reader.lastRange());

  child->addProperty(checksum);

  /*
        child->setSummary(src->summary() + " -> " + dst->summary());
        */
  child->setPayload(reader.slice(lengthNumber - 8));
  result->child = child;
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
