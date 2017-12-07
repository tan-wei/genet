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

namespace plugkit {

namespace {
struct ContextData {
  FrameStorePtr store;
  std::unique_ptr<Filter> filter;
  std::vector<RawFrame> frames;
  size_t length = 0;
  size_t offset = 0;
};

RawFrame createRawFrame(const Frame *frame) {
  RawFrame raw;
  raw.data = nullptr;
  raw.length = 0;

  const Layer *root = frame->rootLayer();
  const auto &payloads = root->payloads();

  return raw;

  /*
  auto layer = new Layer(Token_get("[eth]"));
  layer->addTag(Token_get("[eth]"));

  auto payload = new Payload();
  payload->addSlice(Slice{raw.data, raw.data + raw.length});
  layer->addPayload(payload);

  using namespace std::chrono;
  const Timestamp &ts =
      system_clock::from_time_t(raw.tsSec) + nanoseconds(raw.tsNsec);

  auto frame = new Frame();
  frame->setTimestamp(ts);
  frame->setRootLayer(layer);
  frame->setLength(raw.actualLength);
  layer->setFrame(frame);
  return frame;
  */
}

const RawFrame *apiCallback(Context *ctx, size_t *length) {
  ContextData *data = static_cast<ContextData *>(ctx->data);
  size_t size = 1024;
  if (data->offset + size >= data->length) {
    size = data->length - data->offset;
  }
  if (size == 0) {
    return nullptr;
  }
  const std::vector<const FrameView *> views =
      data->store->get(data->offset, size);
  data->frames.resize(size);
  for (size_t i = 0; i < size; ++i) {
    data->frames[i] = createRawFrame(views[i]->frame());
  }
  data->offset += size;
  *length = size;
  return data->frames.data();
}

} // namespace

class FileExporterThread::Private {
public:
  Callback callback;
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

  auto exporters = d->exporters;
  d->thread = std::thread([this, file, filter, exporters]() {
    ContextData data;
    data.store = d->store;
    data.length = d->store->dissectedSize();
    data.filter.reset(new Filter(filter));

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
