#ifndef PLUGKIT_SESSION_H
#define PLUGKIT_SESSION_H

#include "logger.hpp"
#include "token.h"
#include "types.hpp"
#include "variant.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace plugkit {

class Frame;
class FrameView;

struct Dissector;
struct FileImporter;
struct FileExporter;

class Session final {
  friend class SessionFactory;

public:
  struct RawFrame {
    int link = 0;
    Slice payload;
    size_t length = 0;
    Timestamp timestamp = std::chrono::system_clock::now();
    uint32_t sourceId = 0;
  };

  struct Status {
    bool capture = false;
    double importerProgress = 0.0;
    double exporterProgress = 0.0;
  };
  using StatusCallback = std::function<void(const Status &)>;

  struct FilterStatus {
    uint32_t frames = 0;
  };
  using FilterStatusMap = std::unordered_map<std::string, FilterStatus>;
  using FilterCallback = std::function<void(const FilterStatusMap &)>;

  struct FrameStatus {
    uint32_t frames = 0;
  };
  using FrameCallback = std::function<void(const FrameStatus &)>;

  using LoggerCallback = std::function<void(Logger::MessagePtr &&msg)>;
  using InspectorCallback = std::function<void(std::string, std::string)>;

private:
  struct Config;

public:
  Session(const Config &config);
  ~Session();
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;
  std::string networkInterface() const;
  bool promiscuous() const;
  int snaplen() const;

  bool startPcap();
  bool stopPcap();

  void setDisplayFilter(const std::string &name, const std::string &body);
  std::vector<uint32_t> getFilteredFrames(const std::string &name,
                                          uint32_t offset,
                                          uint32_t length) const;
  std::vector<const FrameView *> getFrames(uint32_t offset,
                                           uint32_t length) const;

  int importFile(const std::string &file);
  int exportFile(const std::string &file, const std::string &filter);

  void sendInspectorMessage(const std::string &id, const std::string &msg);
  std::vector<std::string> inspectors() const;

  void setStatusCallback(const StatusCallback &callback);
  void setFilterCallback(const FilterCallback &callback);
  void setFrameCallback(const FrameCallback &callback);
  void setLoggerCallback(const LoggerCallback &callback);
  void setInspectorCallback(const InspectorCallback &callback);

  int id() const;

private:
  class Private;
  Private *d;
};

using SessionPtr = std::shared_ptr<Session>;

typedef enum DissectorType {
  DISSECTOR_PACKET = 0,
  DISSECTOR_STREAM = 1
} DissectorType;

class SessionFactory {
public:
  SessionFactory();
  ~SessionFactory();
  SessionPtr create() const;

  void setNetworkInterface(const std::string &id);
  std::string networkInterface() const;
  void setPromiscuous(bool promisc);
  bool promiscuous() const;
  void setSnaplen(int len);
  int snaplen() const;
  void setBpf(const std::string &filter);
  std::string bpf() const;
  void setOption(const std::string &key, const Variant &value);

  void registerLinkLayer(int link, Token token);
  void registerDissector(const Dissector &diss, DissectorType type);
  void registerDissector(const std::string &script, DissectorType type);
  void registerImporter(const FileImporter &importer);
  void registerExporter(const FileExporter &exporter);

private:
  std::unique_ptr<Session::Config> d;
};
} // namespace plugkit

#endif
