#include "file_exporter_thread.hpp"
#include "context.hpp"
#include "file.h"
#include "filter.hpp"
#include "frame_store.hpp"
#include <thread>
#include <vector>

namespace plugkit {

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

FileExporterThread::~FileExporterThread() {}

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
    Context ctx;
    ctx.data = d->store.get();
    Filter frameFilter(filter);

    for (const FileExporter &exporter : exporters) {
      if (!exporter.func)
        continue;
      FileStatus status =
          exporter.func(&ctx, file.c_str(),
                        [](Context *ctx, size_t *length) -> const RawFrame * {
                          const FrameStore *store =
                              static_cast<const FrameStore *>(ctx->data);
                          *length = store->dissectedSize();
                          return nullptr;
                        });
      if (status != FILE_STATUS_UNSUPPORTED) {
        return;
      }
    }
  });
  return true;
}

} // namespace plugkit
