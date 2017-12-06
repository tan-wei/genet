#ifndef PLUGKIT_FILE_EXPORTER_THREAD_H
#define PLUGKIT_FILE_EXPORTER_THREAD_H

#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Frame;
struct FileExporter;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FileExporterThread final {
public:
  using Callback = std::function<void(double)>;

public:
  FileExporterThread(const FrameStorePtr &store);
  ~FileExporterThread();
  void setCallback(const Callback &callback);
  void addExporter(const FileExporter &exporter);
  bool start(const std::string &file, const std::string &filter);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
