#ifndef PLUGKIT_FILE_IMPORTEER_THREAD_H
#define PLUGKIT_FILE_IMPORTEER_THREAD_H

#include "config_map.hpp"
#include "task.hpp"
#include "token.hpp"
#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Frame;
struct FileImporter;

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class RootAllocator;

class FileImporterTask final : public Task {
public:
  using Callback = std::function<void(int, Frame **, size_t, double)>;

public:
  FileImporterTask(const std::string &file);
  ~FileImporterTask();
  void run(int id) override;
  void setConfigs(const ConfigMap &options);
  void setLogger(const LoggerPtr &logger);
  void setCallback(const Callback &callback);
  void setAllocator(RootAllocator *allocator);
  void registerLinkLayer(int link, Token token);
  void addImporter(const FileImporter &importer);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
