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

enum State { STATE_START, STATE_HEADER, STATE_BODY, STATE_CLOSED };

class Worker {
public:
  Worker(const std::unordered_set<uint16_t> &ports);
  void analyze(Context *ctx, Layer *layer);
  bool analyze_start(Context *ctx, Layer *layer);
  bool analyze_header(Context *ctx, Layer *layer, Layer *child,
                      Property *headers);
  bool analyze_body(Context *ctx, Layer *layer, Layer *child,
                    Property *headers);

public:
  const std::unordered_set<uint16_t> ports;
  size_t offset = 0;
  StreamReader *reader;

  State state = STATE_START;
  size_t headerLength = 0;
  int64_t contentLength = -1;
  std::string contentType;
};

Worker::Worker(const std::unordered_set<uint16_t> &ports) : ports(ports) {}

bool Worker::analyze_start(Context *ctx, Layer *layer) {
  uint16_t srcPort = Property_uint64(Layer_propertyFromId(layer, srcToken));
  uint16_t dstPort = Property_uint64(Layer_propertyFromId(layer, dstToken));

  if (!ports.empty() && ports.find(srcPort) == ports.end() &&
      ports.find(dstPort) == ports.end()) {
    state = STATE_CLOSED;
  } else {
    state = STATE_HEADER;
  }
  return false;
}

bool Worker::analyze_header(Context *ctx, Layer *layer, Layer *child,
                            Property *headers) {
  Range range = StreamReader_search(reader, "\r\n", 2, offset);

  if (range.end == 0)
    return false;
  size_t length = range.begin - offset;

  if (length == 0) {
    headerLength = range.end;
    state = STATE_BODY;
    return false;
  }

  std::unique_ptr<char[]> buf(new char[length]);
  const Slice &slice = StreamReader_read(reader, &buf[0], length, offset);

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
    Variant *value = Property_mapValueRef(headers, keyBegin, keyEnd - keyBegin);
    Variant_setString(value, valueBegin, valueEnd - valueBegin);
    if (strncmp("Content-Length", keyBegin, keyEnd - keyBegin) == 0) {
      contentLength =
          std::stoll(std::string(valueBegin, valueEnd - valueBegin));
    } else if (strncmp("Content-Type", keyBegin, keyEnd - keyBegin) == 0) {
      contentType = std::string(valueBegin, valueEnd - valueBegin);
    }
  }

  offset = range.end;
  return true;
}

bool Worker::analyze_body(Context *ctx, Layer *layer, Layer *child,
                          Property *headers) {
  size_t httpLength = contentLength + headerLength;
  if (contentLength >= 0 && StreamReader_length(reader) >= httpLength) {

    size_t bodyOffset = headerLength;
    Payload *chunk = Layer_addPayload(child);
    Payload_setType(chunk, Token_get(("@mime:" + contentType).c_str()));

    while (bodyOffset < httpLength) {
      const Slice &slice = StreamReader_read(
          reader, nullptr, httpLength - bodyOffset, bodyOffset);
      Payload_addSlice(chunk, slice);
      bodyOffset += Slice_length(slice);
    }

    offset = headerLength + contentLength;
    headers = nullptr;
    state = STATE_HEADER;
  }
  return false;
}

void Worker::analyze(Context *ctx, Layer *layer) {
  if (state == STATE_CLOSED) {
    return;
  }

  size_t payloads = 0;
  auto begin = Layer_payloads(layer, &payloads);
  for (size_t i = 0; i < payloads; ++i) {
    if (Payload_type(begin[i]) == reassembledToken) {
      StreamReader_addPayload(reader, begin[i]);
    }
  }

  Layer *child = Layer_addLayer(layer, httpToken);
  Layer_addTag(child, httpToken);
  Property *headers = Layer_addProperty(child, headersToken);

  State prevState = state;
  while (1) {
    bool next = false;

    switch (state) {
    case STATE_START:
      next = analyze_start(ctx, layer);
      break;
    case STATE_HEADER:
      next = analyze_header(ctx, layer, child, headers);
      break;
    case STATE_BODY:
      next = analyze_body(ctx, layer, child, headers);
      break;
    case STATE_CLOSED:
      return;
    }

    if (!next && prevState == state)
      return;
    prevState = state;
  }
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_STREAM);
  Dissector_addLayerHint(diss, Token_get("tcp-stream"));
  Dissector_setAnalyzer(diss, [](Context *ctx, void *data, Layer *layer) {
    Worker *worker = static_cast<Worker *>(data);
    worker->analyze(ctx, layer);
  });
  Dissector_setWorkerFactory(
      diss,
      [](Context *ctx) -> void * {
        auto httpPorts = Variant_mapValue(
            Variant_mapValue(Context_options(ctx), "dissector-essentials", -1),
            "httpPorts", -1);

        const Variant *value = nullptr;
        std::unordered_set<uint16_t> ports;
        for (size_t i = 0; (value = Variant_arrayValue(httpPorts, i)); ++i) {
          ports.insert(Variant_uint64(value));
        }
        Worker *worker = new Worker(ports);
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
