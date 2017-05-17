#ifndef PLUGKIT_SESSION_H
#define PLUGKIT_SESSION_H

#include <memory>
#include <vector>
#include <string>
#include "variant.hpp"
#include "logger.hpp"

namespace plugkit {

class Frame;

class FrameView;
using FrameViewConstPtr = std::shared_ptr<const FrameView>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class StreamDissectorFactory;
using StreamDissectorFactoryConstPtr =
    std::shared_ptr<const StreamDissectorFactory>;

class Session final {
  friend class SessionFactory;

public:
  struct RawFrame {
    int link = 0;
    Slice payload;
    size_t length = 0;
    Variant::Timestamp timestamp = std::chrono::system_clock::now();
    uint32_t sourceId = 0;
  };

  struct Status {
    bool capture = false;
  };
  using StatusCallback = std::function<void(const Status &)>;

  struct FilterStatus {
    uint32_t frames = 0;
  };
  using FilterStatusMap = std::unordered_map<std::string, FilterStatus>;
  using FilterCallback = std::function<void(const FilterStatusMap &)>;

  struct FrameStatus {
    uint32_t frames = 0;
    uint32_t queue = 0;
  };
  using FrameCallback = std::function<void(const FrameStatus &)>;

  using LoggerCallback = std::function<void(Logger::MessagePtr &&msg)>;

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
  Variant options() const;

  bool startPcap();
  bool stopPcap();

  void setDisplayFilter(const std::string &name, const std::string &body);
  std::vector<uint32_t> getFilteredFrames(const std::string &name,
                                          uint32_t offset,
                                          uint32_t length) const;
  std::vector<FrameViewConstPtr> getFrames(uint32_t offset,
                                           uint32_t length) const;

  void analyze(const std::vector<RawFrame> &rawFrames);

  void setStatusCallback(const StatusCallback &callback);
  void setFilterCallback(const FilterCallback &callback);
  void setFrameCallback(const FrameCallback &callback);
  void setLoggerCallback(const LoggerCallback &callback);

  int id() const;

private:
  class Private;
  Private *d;
};

using SessionPtr = std::shared_ptr<Session>;

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
  void setOptions(const Variant &options);
  Variant options() const;

  void registerLinkLayer(int link, const std::string &ns,
                         const std::string &name);
  void registerDissector(const DissectorFactoryConstPtr &factory);
  void registerStreamDissector(const StreamDissectorFactoryConstPtr &factory);

private:
  std::unique_ptr<Session::Config> d;
};
}

#endif
