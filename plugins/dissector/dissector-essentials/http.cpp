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
const auto headersToken = Token_get("http.headers");
const auto reassembledToken = Token_get("@reassembled");

struct Worker {
  std::unordered_set<uint16_t> ports;
  bool closed = false;
  bool header = false;
  size_t offset = 0;
  StreamReader *reader;
};

void analyze(Context *ctx, void *data, Layer *layer) {
  Worker *worker = static_cast<Worker *>(data);
  if (worker->closed) {
    return;
  }

  uint16_t srcPort = Property_uint64(Layer_propertyFromId(layer, srcToken));
  uint16_t dstPort = Property_uint64(Layer_propertyFromId(layer, dstToken));

  const auto &ports = worker->ports;
  if (!ports.empty() && ports.find(srcPort) == ports.end() &&
      ports.find(dstPort) == ports.end()) {
    return;
  }

  size_t payloads = 0;
  auto begin = Layer_payloads(layer, &payloads);
  for (size_t i = 0; i < payloads; ++i) {
    if (Payload_type(begin[i]) == reassembledToken) {
      StreamReader_addPayload(worker->reader, begin[i]);
    }
  }

  if (worker->header)
    return;

  Layer *child = Layer_addLayer(layer, httpToken);
  Layer_addTag(child, httpToken);

  Property *headers = Layer_addProperty(child, headersToken);

  while (1) {
    Range range =
        StreamReader_search(worker->reader, "\r\n", 2, worker->offset);

    if (range.end > 0) {
      size_t length = range.begin - worker->offset;
      if (length == 0) {
        worker->header = true;
        break;
      }

      std::unique_ptr<char[]> buf(new char[length]);
      const Slice &slice =
          StreamReader_read(worker->reader, &buf[0], length, worker->offset);

      const char *keyBegin = nullptr;
      const char *keyEnd = nullptr;
      const char *valueBegin = nullptr;
      const char *valueEnd = nullptr;
      for (const char *ptr = slice.begin; ptr < slice.end; ++ptr) {
        char c = *ptr;
        if (!keyBegin && c != ' ') {
          keyBegin = ptr;
          continue;
        }
        if (!keyEnd && c == ':') {
          keyEnd = ptr;
          continue;
        }
        if (keyEnd && !valueBegin && c != ' ') {
          valueBegin = ptr;
          continue;
        }
      }
      if (valueBegin) {
        for (const char *ptr = slice.end - 1; ptr >= valueBegin; --ptr) {
          char c = *ptr;
          if (c != ' ') {
            valueEnd = ptr + 1;
            break;
          }
        }
      }

      if (keyBegin && keyEnd && valueBegin) {
        Variant *value =
            Property_mapValueRef(headers, keyBegin, keyEnd - keyBegin);
        Variant_setString(value, valueBegin, valueEnd - valueBegin);
      }

      worker->offset = range.end;
    } else {
      break;
    }
  }
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_STREAM);
  Dissector_addLayerHint(diss, Token_get("tcp-stream"));
  Dissector_setAnalyzer(diss, analyze);
  Dissector_setWorkerFactory(
      diss,
      [](Context *ctx) -> void * {
        auto httpPorts = Variant_mapValue(
            Variant_mapValue(Context_options(ctx), "dissector-essentials", -1),
            "httpPorts", -1);

        const Variant *value = nullptr;
        Worker *worker = new Worker();
        for (size_t i = 0; (value = Variant_arrayValue(httpPorts, i)); ++i) {
          worker->ports.insert(Variant_uint64(value));
        }
        worker->reader = StreamReader_create();
        return worker;
      },
      [](Context *ctx, void *data) {
        Worker *worker = static_cast<Worker *>(data);
        StreamReader_destroy(worker->reader);
        delete worker;
      });
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}
}

NODE_MODULE(dissectorEssentials, Init);
