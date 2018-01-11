#include "file_importer_thread.hpp"
#include "context.hpp"
#include "file.h"
#include "frame.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include <chrono>
#include <thread>
#include <vector>

namespace plugkit {

namespace {

struct CallbackData {
  FileImporterThread::Callback callback;
  std::unordered_map<int, Token> linkLayers;
};

Frame *createFrame(Context *ctx, Token tag, const RawFrame &raw) {
  auto layer = Context_allocLayer(ctx, tag);
  layer->addTag(tag);

  auto payload = Context_allocPayload(ctx);
  payload->addSlice(Slice{raw.data, raw.data + raw.length});
  payload->setRange(Range{0, raw.length});
  layer->addPayload(payload);

  using namespace std::chrono;
  const Timestamp &ts =
      system_clock::from_time_t(raw.tsSec) + nanoseconds(raw.tsNsec);

  auto frame = Context_allocFrame(ctx);
  frame->setTimestamp(ts);
  frame->setRootLayer(layer);
  frame->setLength(raw.actualLength);
  layer->setFrame(frame);
  return frame;
}

bool apiCallback(Context *ctx,
                 const RawFrame *begin,
                 size_t length,
                 double progress) {
  std::vector<Frame *> frames;
  frames.reserve(length);

  const CallbackData *data = static_cast<const CallbackData *>(ctx->data);

  for (size_t i = 0; i < length; ++i) {
    const RawFrame &raw = begin[i];
    const auto &linkLayer = data->linkLayers.find(raw.link);
    Token tag = Token_get("[unknown]");
    if (linkLayer != data->linkLayers.end()) {
      tag = linkLayer->second;
    }
    frames.push_back(createFrame(ctx, tag, raw));
  }

  data->callback(frames.data(), frames.size(), progress);
  return true;
}

} // namespace

class FileImporterThread::Private {
public:
  int counter = 0;
  Callback callback;
  std::unordered_map<int, Token> linkLayers;
  std::vector<FileImporter> importers;
  std::thread thread;
  RootAllocator *allocator = nullptr;
};

FileImporterThread::FileImporterThread() : d(new Private()) {}

FileImporterThread::~FileImporterThread() {
  if (d->thread.joinable()) {
    d->thread.join();
  }
}

void FileImporterThread::setAllocator(RootAllocator *allocator) {
  d->allocator = allocator;
}

void FileImporterThread::setCallback(const Callback &callback) {
  d->callback = callback;
}

void FileImporterThread::registerLinkLayer(int link, Token token) {
  d->linkLayers[link] = token;
}

void FileImporterThread::addImporter(const FileImporter &importer) {
  d->importers.push_back(importer);
}

int FileImporterThread::start(const std::string &file) {
  if (d->thread.joinable())
    return -1;

  d->callback(nullptr, 0, 0.0);

  auto importers = d->importers;
  d->thread = std::thread([this, file, importers]() {
    Context ctx;
    CallbackData data;
    data.callback = d->callback;
    data.linkLayers = d->linkLayers;
    ctx.rootAllocator = d->allocator;
    ctx.data = &data;
    for (const FileImporter &importer : importers) {
      if (!importer.func)
        continue;
      FileStatus status = importer.func(&ctx, file.c_str(), apiCallback);
      if (status != FILE_STATUS_UNSUPPORTED) {
        return;
      }
    }
  });
  return d->counter++;
}

} // namespace plugkit
