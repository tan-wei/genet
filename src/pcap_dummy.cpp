#include "pcap_dummy.hpp"
#include "private/frame.hpp"
#include "layer.hpp"
#include "slice.hpp"
#include "stream_logger.hpp"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>

namespace plugkit {

class PcapDummy::Private {
public:
  Private();
  ~Private();

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  Callback callback;
  std::unordered_map<int, std::pair<std::string, std::string>> linkLayers;
  int link;

  std::mutex mutex;
  std::thread thread;
  std::string networkInterface;
  bool promiscuous = false;
  int snaplen = 2048;

  bool closed = false;
};

PcapDummy::Private::Private() {}

PcapDummy::Private::~Private() {}

PcapDummy::PcapDummy(int link) : d(new Private()) { d->link = link; }

PcapDummy::~PcapDummy() { stop(); }

void PcapDummy::setLogger(const LoggerPtr &logger) { d->logger = logger; }

void PcapDummy::setCallback(const Callback &callback) {
  d->callback = callback;
}

void PcapDummy::setNetworkInterface(const std::string &id) {
  d->networkInterface = id;
}

std::string PcapDummy::networkInterface() const { return d->networkInterface; }

void PcapDummy::setPromiscuous(bool promisc) { d->promiscuous = promisc; }

bool PcapDummy::promiscuous() const { return d->promiscuous; }

void PcapDummy::setSnaplen(int len) { d->snaplen = len; }

int PcapDummy::snaplen() const { return d->snaplen; }

bool PcapDummy::setBpf(const std::string &filter) { return true; }

bool PcapDummy::start() {
  if (d->thread.joinable())
    return false;

  std::string ns = "<unknown>";
  std::string name = "Unknown";
  const auto &link = d->linkLayers.find(d->link);
  if (link != d->linkLayers.end()) {
    ns = link->second.first;
    name = link->second.second;
  }

  d->thread = std::thread([this, ns, name]() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(d->mutex);
        if (d->closed)
          return;
        if (d->callback) {
          auto layer = std::make_shared<Layer>();
          layer->setNs(ns);
          layer->setName(name);
          layer->setPayload(Slice());

          auto frame = Frame::Private::create();
          frame->d->setLength(125);
          frame->d->setRootLayer(layer);

          d->callback(std::move(frame));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
    }
  });

  return true;
}

bool PcapDummy::stop() {
  if (!d->thread.joinable())
    return false;
  {
    std::lock_guard<std::mutex> lock(d->mutex);
    d->closed = true;
  }
  if (d->thread.joinable())
    d->thread.join();
  d->closed = false;
  return true;
}

std::vector<NetworkInterface> PcapDummy::devices() const {
  std::vector<NetworkInterface> devs;
  return devs;
}

bool PcapDummy::hasPermission() const { return true; }

bool PcapDummy::running() const { return d->thread.joinable(); }

void PcapDummy::registerLinkLayer(int link, const std::string &ns,
                                  const std::string &name) {
  d->linkLayers[link] = std::make_pair(ns, name);
}
}
