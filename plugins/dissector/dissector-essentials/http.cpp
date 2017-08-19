#include <list>
#include <algorithm>
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

struct Worker {};

void analyze(Context *ctx, void *data, Layer *layer) {}

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
        size_t len;
        Variant_array(httpPorts, &len);
        printf("XXX %d\n", len);
        return new Worker();
      },
      [](Context *ctx, void *data) { delete static_cast<Worker *>(data); });
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}
}

NODE_MODULE(dissectorEssentials, Init);
