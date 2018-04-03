#ifndef PLUGKIT_FILE_IMPORTEER_THREAD_H
#define PLUGKIT_FILE_IMPORTEER_THREAD_H

#include "task.hpp"
#include "token.hpp"
#include <functional>
#include <memory>
#include <string>

namespace plugkit {

class Frame;
struct FileImporter;

class SessionContext;

class FileImporterTask final : public Task {
public:
  using Callback = std::function<void(int, Frame **, size_t, double)>;

public:
  FileImporterTask(const SessionContext *sctx, const std::string &file);
  ~FileImporterTask();
  void run(int id) override;
  void setCallback(const Callback &callback);
  void registerLinkLayer(int link, Token token);
  void addImporter(const FileImporter &importer);

private:
  class Private;
  std::unique_ptr<Private> d;
};

} // namespace plugkit

#endif
