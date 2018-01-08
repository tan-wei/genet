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
#include "http_parser.h"

#define PLUGKIT_ENABLE_LOGGING
#include <plugkit/logger.h>

using namespace plugkit;

namespace {

const auto httpToken = Token_get("http");
const auto headersToken = Token_get("http.headers");
const auto streamToken = Token_get("@stream");

class HTTPWorker {
public:
  HTTPWorker();
  ~HTTPWorker();
  void analyze(Context *ctx, Layer *layer);

private:
  http_parser_settings settings;
  http_parser parser;
};

HTTPWorker::HTTPWorker()
{
  http_parser_init(&parser, HTTP_RESPONSE);
  parser.data = this;
}

HTTPWorker::~HTTPWorker()
{

}

void HTTPWorker::analyze(Context *ctx, Layer *layer) {
  size_t payloads = 0;
  auto begin = Layer_payloads(layer, &payloads);
  for (size_t i = 0; i < payloads; ++i) {
    if (Payload_type(begin[i]) == streamToken) {
      size_t nslices = 0;
      const Slice *slices = Payload_slices(begin[i], &nslices);
      for (size_t j = 0; j < nslices; ++j) {
        size_t npersed = http_parser_execute(&parser, &settings,
          slices[j].begin, Slice_length(slices[j]));
        printf(">> %d %d \n", npersed, Slice_length(slices[j]));
      }
    }
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
