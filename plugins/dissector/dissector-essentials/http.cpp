#include <unordered_set>
#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>

#define PLUGKIT_ENABLE_LOGGING
#include <plugkit/logger.h>

using namespace plugkit;

namespace {

const auto httpToken = Token_get("http");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");

struct Worker {
  std::unordered_set<uint16_t> ports;
};

void analyze(Context *ctx, void *data, Layer *layer) {
  Worker *worker = static_cast<Worker *>(data);
  uint16_t srcPort = Property_uint64(Layer_propertyFromId(layer, srcToken));
  uint16_t dstPort = Property_uint64(Layer_propertyFromId(layer, dstToken));

  const auto &ports = worker->ports;
  if (!ports.empty() && ports.find(srcPort) == ports.end() &&
      ports.find(dstPort) == ports.end()) {
    return;
  }
  Layer *child = Layer_addLayer(layer, httpToken);
  Layer_addTag(child, httpToken);
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_STREAM);
  Dissector_addLayerHint(diss, Token_get("tcp-stream"));
  Dissector_setAnalyzer(diss, analyze);
  Dissector_setWorkerFactory(
      diss,
      [](Context *ctx) -> void * {
        auto httpPorts = Variant_mapValue(
            Variant_mapValue(Context_options(ctx), "dissector-essentials"),
            "httpPorts");

        const Variant *value = nullptr;
        Worker *worker = new Worker();
        for (size_t i = 0; (value = Variant_valueAt(httpPorts, i)); ++i) {
          worker->ports.insert(Variant_uint64(value));
        }
        return worker;
      },
      [](Context *ctx, void *data) { delete static_cast<Worker *>(data); });
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}
}

NODE_MODULE(dissectorEssentials, Init);
