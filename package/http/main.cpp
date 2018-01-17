#include "http_parser.h"
#include <plugkit/attr.h>
#include <plugkit/context.h>
#include <plugkit/dissector.h>
#include <plugkit/layer.h>
#include <plugkit/module.h>
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
const auto enumToken = Token_get("@enum");
const auto novalueToken = Token_get("@novalue");
const auto httpToken = Token_get("http");
const auto versionToken = Token_get("http.version");
const auto pathToken = Token_get("http.path");
const auto statusToken = Token_get("http.status");
const auto methodToken = Token_get("http.method");
const auto headersToken = Token_get("http.headers");
const auto keepAliveToken = Token_get("http.keepAlive");
const auto upgradeToken = Token_get("http.upgrade");
const auto streamToken = Token_get("@stream");

const Token methodTable[] = {
    Token_get("http.method.delete"),     Token_get("http.method.get"),
    Token_get("http.method.head"),       Token_get("http.method.post"),
    Token_get("http.method.put"),        Token_get("http.method.connect"),
    Token_get("http.method.options"),    Token_get("http.method.trace"),
    Token_get("http.method.copy"),       Token_get("http.method.lock"),
    Token_get("http.method.mocol"),      Token_get("http.method.move"),
    Token_get("http.method.propfind"),   Token_get("http.method.proppatch"),
    Token_get("http.method.search"),     Token_get("http.method.unlock"),
    Token_get("http.method.bind"),       Token_get("http.method.rebind"),
    Token_get("http.method.unbind"),     Token_get("http.method.acl"),
    Token_get("http.method.report"),     Token_get("http.method.mkactivity"),
    Token_get("http.method.checkout"),   Token_get("http.method.merge"),
    Token_get("http.method.msearch"),    Token_get("http.method.notify"),
    Token_get("http.method.subscribe"),  Token_get("http.method.unsubscribe"),
    Token_get("http.method.patch"),      Token_get("http.method.purge"),
    Token_get("http.method.mkcalender"), Token_get("http.method.link"),
    Token_get("http.method.unlink"),
};

static const std::unordered_map<int, Token> statusTable = {
    {100, Token_get("http.status.continue")},
    {101, Token_get("http.status.switchingProtocols")},
    {102, Token_get("http.status.processing")},
    {200, Token_get("http.status.ok")},
    {201, Token_get("http.status.created")},
    {202, Token_get("http.status.accepted")},
    {203, Token_get("http.status.nonAuthoritativeInformation")},
    {204, Token_get("http.status.noContent")},
    {205, Token_get("http.status.resetContent")},
    {206, Token_get("http.status.partialContent")},
    {207, Token_get("http.status.multiStatus")},
    {208, Token_get("http.status.alreadyReported")},
    {226, Token_get("http.status.imUsed")},
    {300, Token_get("http.status.multipleChoices")},
    {301, Token_get("http.status.movedPermanently")},
    {302, Token_get("http.status.found")},
    {303, Token_get("http.status.seeOther")},
    {304, Token_get("http.status.notModified")},
    {305, Token_get("http.status.useProxy")},
    {307, Token_get("http.status.temporaryRedirect")},
    {308, Token_get("http.status.permanentRedirect")},
    {400, Token_get("http.status.badRequest")},
    {401, Token_get("http.status.unauthorized")},
    {402, Token_get("http.status.paymentRequired")},
    {403, Token_get("http.status.forbidden")},
    {404, Token_get("http.status.notFound")},
    {405, Token_get("http.status.methodNotAllowed")},
    {406, Token_get("http.status.notAcceptable")},
    {407, Token_get("http.status.proxyAuthenticationRequired")},
    {408, Token_get("http.status.requestTimeout")},
    {409, Token_get("http.status.conflict")},
    {410, Token_get("http.status.gone")},
    {411, Token_get("http.status.lengthRequired")},
    {412, Token_get("http.status.preconditionFailed")},
    {413, Token_get("http.status.payloadTooLarge")},
    {414, Token_get("http.status.uriTooLong")},
    {415, Token_get("http.status.unsupportedMediaType")},
    {416, Token_get("http.status.rangeNotSatisfiable")},
    {417, Token_get("http.status.expectationFailed")},
    {421, Token_get("http.status.misdirectedRequest")},
    {422, Token_get("http.status.unprocessableEntity")},
    {423, Token_get("http.status.locked")},
    {424, Token_get("http.status.failedDependency")},
    {426, Token_get("http.status.upgradeRequired")},
    {428, Token_get("http.status.preconditionRequired")},
    {429, Token_get("http.status.tooManyRequests")},
    {431, Token_get("http.status.requestHeaderFieldsTooLarge")},
    {451, Token_get("http.status.unavailableForLegalReasons")},
    {500, Token_get("http.status.internalServerError")},
    {501, Token_get("http.status.notImplemented")},
    {502, Token_get("http.status.badGateway")},
    {503, Token_get("http.status.serviceUnavailable")},
    {504, Token_get("http.status.gatewayTimeout")},
    {505, Token_get("http.status.httpVersionNotSupported")},
    {506, Token_get("http.status.variantAlsoNegotiates")},
    {507, Token_get("http.status.insufficientStorage")},
    {508, Token_get("http.status.loopDetected")},
    {510, Token_get("http.status.notExtended")},
    {511, Token_get("http.status.networkAuthenticationRequired")},
};

class HTTPWorker {
public:
  HTTPWorker();
  ~HTTPWorker();
  void analyze(Context *ctx, Layer *layer);

private:
  class Stream {
  public:
    Stream();
    void analyze(Context *ctx, Layer *layer);
    void parse(const char *data, size_t length);
    void ensureLayer();

    int on_message_begin();
    int on_url(const char *at, size_t length);
    int on_status(const char *at, size_t length);
    int on_header_field(const char *at, size_t length);
    int on_header_value(const char *at, size_t length);
    int on_headers_complete();
    int on_body(const char *at, size_t length);
    int on_message_complete();
    int on_chunk_header();
    int on_chunk_complete();

  private:
    enum class State { HttpUnknown, HttpRequest, HttpResponse, HttpError };

  private:
    http_parser_settings settings;
    http_parser reqParser;
    http_parser resParser;
    State state = State::HttpUnknown;
    Context *lastCtx = nullptr;
    Layer *parent = nullptr;
    Layer *child = nullptr;
  };

private:
  std::unordered_map<uint32_t, Stream> streams;
};

HTTPWorker::Stream::Stream() {
  settings.on_message_begin = [](http_parser *parser) {
    return static_cast<HTTPWorker::Stream *>(parser->data)->on_message_begin();
  };
  ;
  settings.on_url = [](http_parser *parser, const char *at, size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)->on_url(at, length);
  };
  settings.on_status = [](http_parser *parser, const char *at, size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_status(at, length);
  };
  settings.on_header_field = [](http_parser *parser, const char *at,
                                size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_header_field(at, length);
  };
  settings.on_header_value = [](http_parser *parser, const char *at,
                                size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_header_value(at, length);
  };
  settings.on_headers_complete = [](http_parser *parser) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_headers_complete();
  };
  settings.on_body = [](http_parser *parser, const char *at, size_t length) {
    return static_cast<HTTPWorker::Stream *>(parser->data)->on_body(at, length);
  };
  settings.on_message_complete = [](http_parser *parser) {
    return static_cast<HTTPWorker::Stream *>(parser->data)
        ->on_message_complete();
  };
  settings.on_chunk_header = [](http_parser *parser) {
    return static_cast<HTTPWorker::Stream *>(parser->data)->on_chunk_header();
  };
  settings.on_chunk_complete = [](http_parser *parser) {
    return static_cast<HTTPWorker::Stream *>(parser->data)->on_chunk_complete();
  };

  http_parser_init(&reqParser, HTTP_REQUEST);
  reqParser.data = this;
  http_parser_init(&resParser, HTTP_RESPONSE);
  resParser.data = this;
}

void HTTPWorker::Stream::analyze(Context *ctx, Layer *layer) {
  lastCtx = ctx;
  parent = layer;
  if (state == State::HttpError)
    return;

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

  if (state == State::HttpRequest) {
    ensureLayer();
    Attr *version = Layer_addAttr(child, lastCtx, versionToken);
    Attr_setDouble(version, reqParser.http_major + reqParser.http_minor / 10.0);
    Attr *method = Layer_addAttr(child, lastCtx, methodToken);
    Attr_setType(method, enumToken);
    Attr_setUint32(method, reqParser.method);
    Attr *methodValue =
        Layer_addAttr(child, lastCtx, methodTable[reqParser.method]);
    Attr_setType(methodValue, novalueToken);
    Attr_setBool(methodValue, true);

    if (http_should_keep_alive(&reqParser)) {
      Attr *keepAliveValue = Layer_addAttr(child, lastCtx, keepAliveToken);
      Attr_setType(keepAliveValue, novalueToken);
      Attr_setBool(keepAliveValue, true);
    }
    if (reqParser.upgrade) {
      Attr *upgradeValue = Layer_addAttr(child, lastCtx, upgradeToken);
      Attr_setType(upgradeValue, novalueToken);
      Attr_setBool(upgradeValue, true);
    }
  } else if (state == State::HttpResponse) {
    ensureLayer();
    Attr *version = Layer_addAttr(child, lastCtx, versionToken);
    Attr_setDouble(version, resParser.http_major + resParser.http_minor / 10.0);
    Attr *status = Layer_addAttr(child, lastCtx, statusToken);
    Attr_setType(status, enumToken);
    Attr_setUint32(status, resParser.status_code);

    auto statusValueToken = statusTable.find(resParser.status_code);
    if (statusValueToken != statusTable.end()) {
      Attr *statusValue =
          Layer_addAttr(child, lastCtx, statusValueToken->second);
      Attr_setType(statusValue, novalueToken);
      Attr_setBool(statusValue, true);
    }

    if (http_should_keep_alive(&resParser)) {
      Attr *keepAliveValue = Layer_addAttr(child, lastCtx, keepAliveToken);
      Attr_setType(keepAliveValue, novalueToken);
      Attr_setBool(keepAliveValue, true);
    }
    if (resParser.upgrade) {
      Attr *upgradeValue = Layer_addAttr(child, lastCtx, upgradeToken);
      Attr_setType(upgradeValue, novalueToken);
      Attr_setBool(upgradeValue, true);
    }
  }

  child = nullptr;
}

void HTTPWorker::Stream::ensureLayer() {
  if (!child) {
    child = Layer_addLayer(parent, lastCtx, httpToken);
    Layer_addTag(child, httpToken);
  }
}

int HTTPWorker::Stream::on_message_begin() { return 0; }

int HTTPWorker::Stream::on_url(const char *at, size_t length) {
  ensureLayer();
  Attr *attr = Layer_addAttr(child, lastCtx, pathToken);
  Attr_setString(attr, at, length);
  return 0;
}

int HTTPWorker::Stream::on_status(const char *at, size_t length) { return 0; }

int HTTPWorker::Stream::on_header_field(const char *at, size_t length) {
  return 0;
}

int HTTPWorker::Stream::on_header_value(const char *at, size_t length) {
  return 0;
}

int HTTPWorker::Stream::on_headers_complete() { return 0; }

int HTTPWorker::Stream::on_body(const char *at, size_t length) { return 0; }

int HTTPWorker::Stream::on_message_complete() { return 0; }

int HTTPWorker::Stream::on_chunk_header() { return 0; }

int HTTPWorker::Stream::on_chunk_complete() { return 0; }

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

} // namespace

PLUGKIT_MODULE([]() {
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
  return &diss;
})
