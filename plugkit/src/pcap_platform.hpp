#ifndef PLUGKIT_DARWIN_PCAP_PLATFORM_HPP
#define PLUGKIT_DARWIN_PCAP_PLATFORM_HPP

#include "pcap.hpp"

namespace plugkit {

class PcapPlatform final : public Pcap {
public:
  PcapPlatform();
  ~PcapPlatform();
  PcapPlatform(const PcapPlatform &) = delete;
  PcapPlatform &operator=(const PcapPlatform &) = delete;

  void setLogger(const LoggerPtr &logger) override;
  void setCallback(const Callback &callback) override;

  void setNetworkInterface(const std::string &id) override;
  std::string networkInterface() const override;
  void setPromiscuous(bool promisc) override;
  bool promiscuous() const override;
  void setSnaplen(int len) override;
  int snaplen() const override;
  bool setBpf(const std::string &filter) override;

  std::vector<NetworkInterface> devices() const override;
  bool hasPermission() const override;
  bool running() const override;

  void registerLinkLayer(int link, const strns &ns) override;

  bool start() override;
  bool stop() override;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
