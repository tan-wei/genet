#include "file_exporter_thread.hpp"
#include "context.hpp"
#include "file.h"
#include "filter.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include <thread>
#include <vector>
#include <chrono>

namespace plugkit {

namespace {
struct ContextData {
  FrameStorePtr store;
  FileExporterThread::Callback callback;
  std::unordered_map<Token, int> linkLayers;
  std::unique_ptr<Filter> filter;
  std::vector<RawFrame> frames;
  size_t length = 0;
  size_t offset = 0;
};

RawFrame createRawFrame(const ContextData *data, const Frame *frame) {
  RawFrame raw;
  raw.data = nullptr;
  raw.length = 0;

  const Layer *root = frame->rootLayer();
  const auto &payloads = root->payloads();
  if (!payloads.empty()) {
    const auto &slices = payloads[0]->slices();
    if (!slices.empty()) {
      const auto &slice = slices[0];
      raw.data = slice.begin;
      raw.length = slice.end - slice.begin;
    }
  }

  raw.link = 0;
  auto it = data->linkLayers.find(root->id());
  if (it != data->linkLayers.end()) {
    raw.link = it->second;
  }

  auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
    frame->timestamp().time_since_epoch()).count();

  raw.actualLength = frame->length();
  raw.tsSec = nano / 1000000000;
  raw.tsNsec = nano % 1000000000;
  raw.root = frame->rootLayer();
  return raw;
}

const RawFrame *apiCallback(Context *ctx, size_t *length) {
  ContextData *data = static_cast<ContextData *>(ctx->data);
  size_t size = 1024;
  if (data->offset + size >= data->length) {
    size = data->length - data->offset;
  }
  *length = size;
  data->callback(1.0 * data->offset / data->length);
  if (size == 0) {
    return nullptr;
  }
  const std::vector<const FrameView *> views =
      data->store->get(data->offset, size);
  data->frames.resize(size);
  for (size_t i = 0; i < size; ++i) {
    data->frames[i] = createRawFrame(data, views[i]->frame());
  }
  data->offset += size;
  return data->frames.data();
}

} // namespace

class FileExporterThread::Private {
public:
  Callback callback;
  std::unordered_map<Token, int> linkLayers;
  std::vector<FileExporter> exporters;
  std::thread thread;
  FrameStorePtr store;
};

FileExporterThread::FileExporterThread(const FrameStorePtr &store)
    : d(new Private()) {
  d->store = store;
}

FileExporterThread::~FileExporterThread() {
  if (d->thread.joinable()) {
    d->thread.join();
  }
}

void FileExporterThread::registerLinkLayer(Token token, int link) {
  d->linkLayers[token] = link;
}

void FileExporterThread::setCallback(const Callback &callback) {
  d->callback = callback;
}

void FileExporterThread::addExporter(const FileExporter &exporters) {
  d->exporters.push_back(exporters);
}

bool FileExporterThread::start(const std::string &file,
                               const std::string &filter) {
  if (d->thread.joinable())
    return false;

  d->callback(0.0);

  if (d->store->dissectedSize() == 0) {
    d->callback(1.0);
    return true;
  }

  auto exporters = d->exporters;
  d->thread = std::thread([this, file, filter, exporters]() {
    ContextData data;
    data.callback = d->callback;
    data.store = d->store;
    data.length = d->store->dissectedSize();
    data.linkLayers = d->linkLayers;

    Context ctx;
    ctx.data = &data;

    for (const FileExporter &exporter : exporters) {
      if (!exporter.func)
        continue;
      FileStatus status = exporter.func(&ctx, file.c_str(), apiCallback);
      if (status != FILE_STATUS_UNSUPPORTED) {
        return;
      }
    }
  });
  return true;
}

} // namespace plugkit
