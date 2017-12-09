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
Frame *createFrame(Context *ctx, const RawFrame &raw) {
  auto layer = Context_allocLayer(ctx, Token_get("[eth]"));
  layer->addTag(Token_get("[eth]"));

  auto payload = Context_allocPayload(ctx);
  payload->addSlice(Slice{raw.data, raw.data + raw.length});
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
  for (size_t i = 0; i < length; ++i) {
    frames.push_back(createFrame(ctx, begin[i]));
  }
  auto callback = *static_cast<const FileImporterThread::Callback *>(ctx->data);
  callback(frames.data(), frames.size(), progress);
  return true;
}

} // namespace

class FileImporterThread::Private {
public:
  Callback callback;
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

void FileImporterThread::addImporter(const FileImporter &importer) {
  d->importers.push_back(importer);
}

bool FileImporterThread::start(const std::string &file) {
  if (d->thread.joinable())
    return false;

  auto importers = d->importers;
  d->thread = std::thread([this, file, importers]() {
    auto threadCallback = d->callback;
    Context ctx;
    ctx.rootAllocator = d->allocator;
    ctx.data = &threadCallback;
    for (const FileImporter &importer : importers) {
      if (!importer.func)
        continue;
      FileStatus status = importer.func(&ctx, file.c_str(), apiCallback);
      if (status != FILE_STATUS_UNSUPPORTED) {
        return;
      }
    }
  });
  return true;
}

} // namespace plugkit
