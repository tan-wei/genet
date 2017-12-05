#ifndef PLUGKIT_FILE_IMPORTEER_THREAD_H
#define PLUGKIT_FILE_IMPORTEER_THREAD_H

#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Frame;
struct FileImporter;

class FileImporterThread final {
public:
  using Callback = std::function<void(Frame *, size_t, double)>;

public:
  FileImporterThread();
  ~FileImporterThread();
  void setCallback(const Callback &callback);
  void addImporter(const FileImporter &importer);
  bool start(const std::string &file);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
