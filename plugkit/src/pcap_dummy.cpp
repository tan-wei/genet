#include "pcap_dummy.hpp"
#include "layer.hpp"
#include "frame.hpp"
#include "payload.hpp"
#include "stream_logger.hpp"
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace plugkit {

class PcapDummy::Private {
public:
  Private();
  ~Private();

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  Callback callback;
  std::unordered_map<int, Token> linkLayers;
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

  Token tag = Token_get("[unknown]");
  const auto &link = d->linkLayers.find(d->link);
  if (link != d->linkLayers.end()) {
    tag = link->second;
  }

  d->thread = std::thread([this, tag]() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(d->mutex);
        if (d->closed)
          return;
        if (d->callback) {
          auto layer = new Layer(tag);
          layer->addTag(tag);
          layer->addPayload(new Payload(Slice{nullptr, nullptr}));

          auto frame = new Frame();
          frame->setLength(125);
          frame->setRootLayer(layer);
          layer->setFrame(frame);

          d->callback(frame);
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

void PcapDummy::registerLinkLayer(int link, Token token) {
  d->linkLayers[link] = token;
}
}
