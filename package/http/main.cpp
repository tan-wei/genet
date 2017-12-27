#include <nan.h>
#include <plugkit/attr.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/stream_reader.h>
#include <plugkit/token.h>
#include <plugkit/variant.h>
#include <unordered_set>

#define PLUGKIT_ENABLE_LOGGING
#include <plugkit/logger.h>

using namespace plugkit;

namespace {

const auto httpToken = Token_get("http");
const auto headersToken = Token_get("http.headers");
const auto reassembledToken = Token_get("@reassembled");
const auto mimeToken = Token_get("@mime");
const auto mimeTypeToken = Token_get(".mimeType");

enum State { STATE_START, STATE_HEADER, STATE_BODY, STATE_CLOSED };

class HTTPWorker {
public:
  HTTPWorker(const std::unordered_set<uint16_t> &ports);
  void analyze(Context *ctx, Layer *layer);
  bool analyze_start(Context *ctx, Layer *layer);
  bool analyze_header(Context *ctx, Layer *layer, Layer *child, Attr *headers);
  bool analyze_body(Context *ctx, Layer *layer, Layer *child, Attr *headers);

public:
  const std::unordered_set<uint16_t> ports;
  size_t offset = 0;
  StreamReader *reader;

  State state = STATE_START;
  size_t headerLength = 0;
  int64_t contentLength = -1;
  std::string contentType;
  bool chunked = false;
};

HTTPWorker::HTTPWorker(const std::unordered_set<uint16_t> &ports)
    : ports(ports) {}

bool HTTPWorker::analyze_start(Context *ctx, Layer *layer) {
  Token layerId = Layer_id(layer);
  uint16_t srcPort = Attr_uint32(Layer_attr(layer, Token_join(layerId, ".src")));
  uint16_t dstPort = Attr_uint32(Layer_attr(layer, Token_join(layerId, ".dst")));

  if (!ports.empty() && ports.find(srcPort) == ports.end() &&
      ports.find(dstPort) == ports.end()) {
    state = STATE_CLOSED;
  } else {
    state = STATE_HEADER;
  }
  return false;
}

bool HTTPWorker::analyze_header(Context *ctx,
                                Layer *layer,
                                Layer *child,
                                Attr *headers) {
  size_t index = StreamReader_search(reader, "\r\n", 2, offset);

  if (index == RANGE_NPOS)
    return false;
  size_t length = index - 2 - offset;

  if (length == 0) {
    headerLength = index;
    offset = index;
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
    Variant *value = Attr_mapValueRef(headers, keyBegin, keyEnd - keyBegin);
    Variant_setString(value, valueBegin, valueEnd - valueBegin);
    if (strncmp("Content-Length", keyBegin, keyEnd - keyBegin) == 0) {
      contentLength =
          std::stoll(std::string(valueBegin, valueEnd - valueBegin));
    } else if (strncmp("Content-Type", keyBegin, keyEnd - keyBegin) == 0) {
      contentType = std::string(valueBegin, valueEnd - valueBegin);
    } else if (strncmp("Transfer-Encoding", keyBegin, keyEnd - keyBegin) == 0) {
      if (strncmp("chunked", valueBegin, valueEnd - valueBegin) == 0) {
        chunked = true;
      }
    }
  }

  offset = index;
  return true;
}

bool HTTPWorker::analyze_body(Context *ctx,
                              Layer *layer,
                              Layer *child,
                              Attr *headers) {
  size_t httpLength = contentLength + headerLength;
  if (chunked) {
    /*
    Range range = StreamReader_search(reader, "\r\n\r\n", 4, offset);
    if (range.begin > offset) {
      size_t length = range.begin - offset;
      std::unique_ptr<char[]> buf(new char[length]);
      const Slice &slice = StreamReader_read(reader, &buf[0], length, offset);

      size_t chunkLength = std::stoll(
          std::string(slice.begin, Slice_length(slice)), nullptr, 16);
      printf("%llu\n", chunkLength);

      offset = range.end + chunkLength;
      const Slice &slice2 = StreamReader_read(reader, &buf[0], 2, offset);
      printf("@ %.2s @\n", slice2.begin);
    }
    */
  } else if (contentLength >= 0 && StreamReader_length(reader) >= httpLength) {
    size_t bodyOffset = headerLength;
    Payload *chunk = Layer_addPayload(ctx, child);
    Payload_setType(chunk, mimeToken);
    if (!contentType.empty()) {
      Attr *mime = Payload_addAttr(ctx, chunk, mimeTypeToken);
      Attr_setString(mime, contentType.c_str());
    }

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

void HTTPWorker::analyze(Context *ctx, Layer *layer) {
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

  Layer *child = Layer_addLayer(ctx, layer, httpToken);
  Layer_addTag(child, httpToken);
  Attr *headers = Layer_addAttr(ctx, child, headersToken);

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
  static Dissector diss;
  diss.layerHints[0] = Token_get("tcp-stream");
  diss.analyze = [](Context *ctx, const Dissector *diss, Worker data,
                    Layer *layer) {
    HTTPWorker *worker = static_cast<HTTPWorker *>(data.data);
    worker->analyze(ctx, layer);
  };
  diss.createWorker = [](Context *ctx, const Dissector *diss) {
    std::unordered_set<uint16_t> ports = {80};
    HTTPWorker *worker = new HTTPWorker(ports);
    worker->reader = StreamReader_create();
    return Worker{worker};
  };
  diss.destroyWorker = [](Context *ctx, const Dissector *diss, Worker data) {
    HTTPWorker *worker = static_cast<HTTPWorker *>(data.data);
    StreamReader_destroy(worker->reader);
  };
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}
} // namespace

NODE_MODULE(dissectorEssentials, Init);
