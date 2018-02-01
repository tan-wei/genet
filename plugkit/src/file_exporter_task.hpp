#ifndef PLUGKIT_FILE_EXPORTER_THREAD_H
#define PLUGKIT_FILE_EXPORTER_THREAD_H

#include "task.hpp"
#include "token.hpp"
#include "variant_map.hpp"
#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Frame;
struct FileExporter;

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class FrameStore;
using FrameStorePtr = std::shared_ptr<FrameStore>;

class FileExporterTask final : public Task {
public:
  using Callback = std::function<void(int, double)>;

public:
  FileExporterTask(const std::string &file,
                   const std::string &filter,
                   const FrameStorePtr &store);
  ~FileExporterTask();
  void run(int id) override;
  void setOptions(const VariantMap &options);
  void setLogger(const LoggerPtr &logger);
  void setCallback(const Callback &callback);
  void addExporter(const FileExporter &exporter);
  void registerLinkLayer(Token token, int link);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
