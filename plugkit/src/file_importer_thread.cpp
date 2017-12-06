#include "file_importer_thread.hpp"
#include "context.hpp"
#include "file.h"
#include <thread>
#include <vector>

namespace plugkit {

class FileImporterThread::Private {
public:
  Callback callback;
  std::vector<FileImporter> importers;
  std::thread thread;
};

FileImporterThread::FileImporterThread() : d(new Private()) {}

FileImporterThread::~FileImporterThread() {}

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
  auto callback = d->callback;
  d->thread = std::thread([file, callback, importers]() {
    Context ctx;
    ctx.data = &callback;
    for (const FileImporter &importer : importers) {
      if (!importer.func)
        continue;
      FileStatus status =
          importer.func(&ctx, file.c_str(),
                        [](Context *ctx, const RawFrame *frames, size_t length,
                           double progress) {
                          auto callback =
                              *static_cast<const Callback *>(ctx->data);
                          callback(nullptr, 0, progress);
                          return true;
                        });
      if (status != FILE_STATUS_UNSUPPORTED) {
        return;
      }
    }
  });
  return true;
}

} // namespace plugkit
