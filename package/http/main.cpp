#include "http_parser.h"
#include <nan.h>
#include <plugkit/attr.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/stream_reader.h>
#include <plugkit/token.h>
#include <plugkit/variant.h>
#include <unordered_map>

#define PLUGKIT_ENABLE_LOGGING
#include <plugkit/logger.h>

using namespace plugkit;

namespace {

const auto streamIdToken = Token_get(".streamId");
const auto httpToken = Token_get("http");
const auto headersToken = Token_get("http.headers");
const auto streamToken = Token_get("@stream");

class HTTPWorker {
public:
  HTTPWorker();
  ~HTTPWorker();
  void analyze(Context *ctx, Layer *layer);

private:
  class Stream;

private:
  std::unordered_map<uint32_t, Stream> streams;
};

class HTTPWorker::Stream {
public:
  Stream();
  void analyze(Context *ctx, Layer *layer);
  void parse(const char *data, size_t length);

  int on_status(const char *at, size_t length);

private:
  enum class State { HttpUnknown, HttpRequest, HttpResponse, HttpError };

private:
  http_parser_settings settings;
  http_parser reqParser;
  http_parser resParser;
  State state = State::HttpUnknown;
  Layer *child = nullptr;
};

HTTPWorker::Stream::Stream() {
  settings.on_message_begin = nullptr;
  settings.on_url = nullptr;
  settings.on_status = [](http_parser *parser, const char *at, size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_status(at, length);
  };
  settings.on_header_field = nullptr;
  settings.on_header_value = nullptr;
  settings.on_headers_complete = nullptr;
  settings.on_body = nullptr;
  settings.on_message_complete = nullptr;
  settings.on_chunk_header = nullptr;
  settings.on_chunk_complete = nullptr;

  http_parser_init(&reqParser, HTTP_REQUEST);
  reqParser.data = this;
  http_parser_init(&resParser, HTTP_RESPONSE);
  resParser.data = this;
}

void HTTPWorker::Stream::analyze(Context *ctx, Layer *layer) {
  size_t payloads = 0;
  auto begin = Layer_payloads(layer, &payloads);
  for (size_t i = 0; i < payloads; ++i) {
    if (Payload_type(begin[i]) == streamToken) {
      size_t nslices = 0;
      const Slice *slices = Payload_slices(begin[i], &nslices);
      for (size_t j = 0; j < nslices; ++j) {
        parse(slices[j].begin, Slice_length(slices[j]));
      }
    }
  }

  if (state == State::HttpRequest || state == State::HttpResponse) {
    child = Layer_addLayer(ctx, layer, httpToken);
    Layer_addTag(child, httpToken);
  }
}

int HTTPWorker::Stream::on_status(const char *at, size_t length) {
  printf("%s\n", std::string(at, length).c_str());
  return 0;
}

void HTTPWorker::Stream::parse(const char *data, size_t length) {
  switch (state) {
  case State::HttpUnknown: {
    size_t parsed = http_parser_execute(&reqParser, &settings, data, length);
    if (parsed == length) {
      state = State::HttpRequest;
    } else {
      parsed = http_parser_execute(&resParser, &settings, data, length);
      if (parsed == length) {
        state = State::HttpResponse;
      } else {
        state = State::HttpError;
      }
    }
  } break;
  case State::HttpRequest: {
    size_t parsed = http_parser_execute(&reqParser, &settings, data, length);
    if (parsed != length) {
      state = State::HttpError;
    }
  } break;
  case State::HttpResponse: {
    size_t parsed = http_parser_execute(&resParser, &settings, data, length);
    if (parsed != length) {
      state = State::HttpError;
    }
  } break;
  default:;
  }
}

HTTPWorker::HTTPWorker() {}

HTTPWorker::~HTTPWorker() {}

void HTTPWorker::analyze(Context *ctx, Layer *layer) {
  const auto layerId = Layer_id(layer);
  uint32_t streamId =
      Attr_uint32(Layer_attr(layer, Token_join(layerId, streamIdToken)));
  streams[streamId].analyze(ctx, layer);
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
    HTTPWorker *worker = new HTTPWorker();
    return Worker{worker};
  };
  diss.destroyWorker = [](Context *ctx, const Dissector *diss, Worker data) {
    HTTPWorker *worker = static_cast<HTTPWorker *>(data.data);
    delete worker;
  };
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}
} // namespace

NODE_MODULE(dissectorEssentials, Init);
