#include "file_exporter_task.hpp"
#include "context.hpp"
#include "file.hpp"
#include "filter.hpp"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "sandbox.hpp"
#include "worker_thread.hpp"
#include <chrono>
#include <thread>
#include <vector>

namespace plugkit {

namespace {
struct ContextData {
  int id;
  FrameStorePtr store;
  FileExporterTask::Callback callback;
  std::unordered_map<Token, int> linkLayers;
  std::vector<FileExporter> exporters;
  std::vector<RawFrame> frames;
  std::string file;
  std::string filterBody;
  Filter *filter = nullptr;
  size_t length = 0;
  size_t offset = 0;
};

RawFrame createRawFrame(const ContextData *data, const Frame *frame) {
  RawFrame raw;
  raw.payload = nullptr;
  raw.length = 0;

  const Layer *root = frame->rootLayer();
  const auto &payloads = root->payloads();
  if (!payloads.empty()) {
    const auto &slices = payloads[0]->slices();
    if (!slices.empty()) {
      const auto &slice = slices[0];
      raw.payload = slice.data;
      raw.length = slice.length;
    }
  }

  raw.link = 0;
  auto it = data->linkLayers.find(root->id());
  if (it != data->linkLayers.end()) {
    raw.link = it->second;
  }

  auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
                  frame->timestamp().time_since_epoch())
                  .count();

  raw.actualLength = frame->length();
  raw.tsSec = nano / 1000000000;
  raw.tsNsec = nano % 1000000000;
  raw.root = frame->rootLayer();
  return raw;
}

const RawFrame *apiCallback(Context *ctx, size_t *length) {
  constexpr size_t blockSize = 1024;
  ContextData *data = static_cast<ContextData *>(ctx->data);

  size_t size = blockSize;
  data->frames.clear();

  while (data->frames.empty()) {
    if (data->offset + size >= data->length) {
      size = data->length - data->offset;
    }

    data->callback(data->id, 1.0 * data->offset / data->length);
    if (size == 0) {
      *length = 0;
      return nullptr;
    }

    const std::vector<const FrameView *> views =
        data->store->get(data->offset, size);

    char results[blockSize];
    if (data->filter) {
      data->filter->test(results, views.data(), views.size());
    }

    data->frames.reserve(size);

    for (size_t i = 0; i < size; ++i) {
      if (!data->filter || results[i]) {
        data->frames.push_back(createRawFrame(data, views[i]->frame()));
      }
    }

    data->offset += size;
  }

  *length = data->frames.size();
  return data->frames.data();
}

class FileExporterWorkerThread : public WorkerThread {
public:
  FileExporterWorkerThread(const SessionContext *sctx, const ContextData &data)
      : ctx(sctx), data(data) {}

  ~FileExporterWorkerThread() {}

  void enter() override {
    if (!data.filterBody.empty()) {
      filter.reset(new Filter(data.filterBody));
    }

    Sandbox::activate(Sandbox::PROFILE_FILE);
  }

  bool loop() override {
    data.filter = filter.get();
    ctx.data = &data;

    for (const FileExporter &exporter : data.exporters) {
      if (exporter.isSupported && exporter.start) {
        if (exporter.isSupported(&ctx, data.file.c_str())) {
          exporter.start(&ctx, data.file.c_str(), apiCallback);
          return false;
        }
      }
    }
    return false;
  }
  void exit() override { filter.reset(); }

private:
  Context ctx;
  ContextData data;
  std::unique_ptr<Filter> filter;
};

} // namespace

class FileExporterTask::Private {
public:
  std::string file;
  std::string filter;
  Callback callback;
  LoggerPtr logger = std::make_shared<StreamLogger>();
  std::unordered_map<Token, int> linkLayers;
  std::unique_ptr<FileExporterWorkerThread> worker;
  std::vector<FileExporter> exporters;
  std::thread thread;
  FrameStorePtr store;
  const SessionContext *sctx;
};

FileExporterTask::FileExporterTask(const SessionContext *sctx,
                                   const std::string &file,
                                   const std::string &filter,
                                   const FrameStorePtr &store)
    : d(new Private()) {
  d->sctx = sctx;
  d->file = file;
  d->filter = filter;
  d->store = store;
}

FileExporterTask::~FileExporterTask() {}

void FileExporterTask::registerLinkLayer(Token token, int link) {
  d->linkLayers[token] = link;
}

void FileExporterTask::setCallback(const Callback &callback) {
  d->callback = callback;
}

void FileExporterTask::addExporter(const FileExporter &exporters) {
  d->exporters.push_back(exporters);
}

void FileExporterTask::run(int id) {
  ContextData data;
  data.id = id;
  data.callback = d->callback;
  data.store = d->store;
  data.length = d->store->dissectedSize();
  data.linkLayers = d->linkLayers;
  data.exporters = d->exporters;
  data.file = d->file;
  data.filterBody = d->filter;

  d->worker.reset(new FileExporterWorkerThread(d->sctx, data));
  d->worker->start();
  d->worker->join();
}

} // namespace plugkit
