#include "pcap_platform.hpp"
#include "layer.hpp"
#include "private/frame.hpp"
#include "slice.hpp"
#include "stream_logger.hpp"
#include <mutex>
#include <pcap.h>
#include <signal.h>
#include <thread>
#include <unordered_map>

#if defined(PLUGKIT_OS_MAC)
#include <SystemConfiguration/SystemConfiguration.h>
#include <dirent.h>
#include <sys/stat.h>
#elif defined(PLUGKIT_OS_LINUX)
#include <sys/capability.h>
#include <unistd.h>
#elif defined(PLUGKIT_OS_WIN)
#include <iphlpapi.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "iphlpapi.lib")
#endif

#ifndef PCAP_NETMASK_UNKNOWN
#define PCAP_NETMASK_UNKNOWN 0xffffffff
#endif

namespace plugkit {

class PcapPlatform::Private {
public:
  Private();
  ~Private();

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  Callback callback;
  std::unordered_map<int, strns> linkLayers;

  std::mutex mutex;
  std::thread thread;
  pcap_t *pcap = nullptr;

  strns ns;

  bpf_program bpf = {0, nullptr};
  std::string networkInterface;
  bool promiscuous = false;
  int snaplen = 2048;

  std::function<decltype(::pcap_freecode)> pcapFreecode;
  std::function<decltype(::pcap_open_live)> pcapOpenLive;
  std::function<decltype(::pcap_compile)> pcapCompile;
  std::function<decltype(::pcap_geterr)> pcapGeterr;
  std::function<decltype(::pcap_close)> pcapClose;
  std::function<decltype(::pcap_setfilter)> pcapSetfilter;
  std::function<decltype(::pcap_datalink)> pcapDatalink;
  std::function<decltype(::pcap_loop)> pcapLoop;
  std::function<decltype(::pcap_breakloop)> pcapBreakloop;
  std::function<decltype(::pcap_findalldevs)> pcapFindalldevs;
  std::function<decltype(::pcap_freealldevs)> pcapFreealldevs;

  bool pcapLoaded = false;
#if defined(PLUGKIT_OS_WIN)
  HMODULE hLib;
#endif
};

PcapPlatform::Private::Private() {
#if defined(PLUGKIT_OS_WIN)
  if (hLib = LoadLibrary("Wpcap.dll")) {
    pcapFreecode = reinterpret_cast<decltype(::pcap_freecode) *>(
        GetProcAddress(hLib, "pcap_freecode"));
    pcapOpenLive = reinterpret_cast<decltype(::pcap_open_live) *>(
        GetProcAddress(hLib, "pcap_open_live"));
    pcapCompile = reinterpret_cast<decltype(::pcap_compile) *>(
        GetProcAddress(hLib, "pcap_compile"));
    pcapGeterr = reinterpret_cast<decltype(::pcap_geterr) *>(
        GetProcAddress(hLib, "pcap_geterr"));
    pcapClose = reinterpret_cast<decltype(::pcap_close) *>(
        GetProcAddress(hLib, "pcap_close"));
    pcapSetfilter = reinterpret_cast<decltype(::pcap_setfilter) *>(
        GetProcAddress(hLib, "pcap_setfilter"));
    pcapDatalink = reinterpret_cast<decltype(::pcap_datalink) *>(
        GetProcAddress(hLib, "pcap_datalink"));
    pcapLoop = reinterpret_cast<decltype(::pcap_loop) *>(
        GetProcAddress(hLib, "pcap_loop"));
    pcapBreakloop = reinterpret_cast<decltype(::pcap_breakloop) *>(
        GetProcAddress(hLib, "pcap_breakloop"));
    pcapFindalldevs = reinterpret_cast<decltype(::pcap_findalldevs) *>(
        GetProcAddress(hLib, "pcap_findalldevs"));
    pcapFreealldevs = reinterpret_cast<decltype(::pcap_freealldevs) *>(
        GetProcAddress(hLib, "pcap_freealldevs"));
    pcapLoaded = true;
  }
#else
  pcapFreecode = ::pcap_freecode;
  pcapOpenLive = ::pcap_open_live;
  pcapCompile = ::pcap_compile;
  pcapGeterr = ::pcap_geterr;
  pcapClose = ::pcap_close;
  pcapSetfilter = ::pcap_setfilter;
  pcapDatalink = ::pcap_datalink;
  pcapLoop = ::pcap_loop;
  pcapBreakloop = ::pcap_breakloop;
  pcapFindalldevs = ::pcap_findalldevs;
  pcapFreealldevs = ::pcap_freealldevs;
  pcapLoaded = true;
#endif
}

PcapPlatform::Private::~Private() {
#if defined(PLUGKIT_OS_WIN)
  FreeLibrary(hLib);
#endif
}

PcapPlatform::PcapPlatform() : d(new Private()) {}

PcapPlatform::~PcapPlatform() { stop(); }

void PcapPlatform::setLogger(const LoggerPtr &logger) { d->logger = logger; }

void PcapPlatform::setCallback(const Callback &callback) {
  d->callback = callback;
}

void PcapPlatform::setNetworkInterface(const std::string &id) {
  d->networkInterface = id;
}

std::string PcapPlatform::networkInterface() const {
  return d->networkInterface;
}

void PcapPlatform::setPromiscuous(bool promisc) { d->promiscuous = promisc; }

bool PcapPlatform::promiscuous() const { return d->promiscuous; }

void PcapPlatform::setSnaplen(int len) { d->snaplen = len; }

int PcapPlatform::snaplen() const { return d->snaplen; }

bool PcapPlatform::setBpf(const std::string &filter) {
  if (!d->pcapLoaded)
    return false;
  d->pcapFreecode(&d->bpf);
  d->bpf.bf_len = 0;
  d->bpf.bf_insns = nullptr;

  if (filter.empty())
    return true;

  char err[PCAP_ERRBUF_SIZE] = {'\0'};
  pcap_t *pcap = d->pcapOpenLive(d->networkInterface.c_str(), d->snaplen,
                                 d->promiscuous, 1, err);
  if (!pcap) {
    d->logger->log(Logger::LEVEL_ERROR, err, "pcap/bpf");
    return false;
  }

  if (d->pcapCompile(pcap, &d->bpf, filter.c_str(), true,
                     PCAP_NETMASK_UNKNOWN) < 0) {
    d->logger->log(Logger::LEVEL_ERROR, d->pcapGeterr(pcap), "pcap/bpf");
    d->pcapClose(pcap);
    return false;
  }

  d->pcapClose(pcap);
  return true;
}

bool PcapPlatform::running() const { return d->thread.joinable(); }

bool PcapPlatform::start() {
  if (!d->pcapLoaded || d->thread.joinable())
    return false;

  std::lock_guard<std::mutex> lock(d->mutex);
  char err[PCAP_ERRBUF_SIZE] = {'\0'};

  d->pcap = d->pcapOpenLive(d->networkInterface.c_str(), d->snaplen,
                            d->promiscuous, 1, err);
  if (!d->pcap) {
    d->logger->log(Logger::LEVEL_ERROR,
                   std::string("pcap_open_live() failed: ") + err, "pcap");
    return false;
  }

  if (d->bpf.bf_len > 0 && d->pcapSetfilter(d->pcap, &d->bpf) < 0) {
    d->logger->log(Logger::LEVEL_ERROR, "pcap_setfilter() failed", "pcap");
    d->pcapClose(d->pcap);
    d->pcap = nullptr;
    return false;
  }

  int link = d->pcapDatalink(d->pcap);
  const auto &linkLayer = d->linkLayers.find(link);
  if (linkLayer != d->linkLayers.end()) {
    d->ns = linkLayer->second;
  } else {
    d->ns = PK_STRNS("?");
  }

  d->thread = std::thread([this]() {
    d->pcapLoop(
        d->pcap,
        0, [](u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
          PcapPlatform &self = *reinterpret_cast<PcapPlatform *>(user);
          if (self.d->callback) {
            // TODO:ALLOC
            auto layer = new Layer();
            layer->setNs(self.d->ns);
            layer->setPayload(
                Slice(reinterpret_cast<const char *>(bytes), h->caplen));

            using namespace std::chrono;
            const Frame::Timestamp &ts =
                system_clock::from_time_t(h->ts.tv_sec) +
                nanoseconds(h->ts.tv_usec * 1000);

            auto frame = Frame::Private::create();
            frame->d->setTimestamp(ts);
            frame->d->setRootLayer(layer);
            frame->d->setLength(h->len);

            self.d->callback(std::move(frame));
          }
        }, reinterpret_cast<u_char *>(this));
    {
      std::lock_guard<std::mutex> lock(d->mutex);
      d->pcapClose(d->pcap);
      d->pcap = nullptr;
    }
  });
  return true;
}

bool PcapPlatform::stop() {
  if (!d->thread.joinable())
    return false;

  {
    std::lock_guard<std::mutex> lock(d->mutex);
    if (d->pcap)
      d->pcapBreakloop(d->pcap);
  }
  if (d->thread.joinable())
    d->thread.join();
  return true;
}

void PcapPlatform::registerLinkLayer(int link, const strns &ns) {
  d->linkLayers[link] = ns;
}

std::vector<NetworkInterface> PcapPlatform::devices() const {
  std::vector<NetworkInterface> devs;
  if (!d->pcapLoaded)
    return devs;

  std::unordered_map<std::string, std::string> descriptions;

#if defined(PLUGKIT_OS_WIN)
  static constexpr int guidLen = 38;
  {
    PMIB_IFTABLE ifTable = nullptr;
    DWORD dwSize = 0;
    if (GetIfTable(nullptr, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
      ifTable = static_cast<MIB_IFTABLE *>(malloc(dwSize));
    }

    if (GetIfTable(ifTable, &dwSize, 0) == NO_ERROR) {
      if (ifTable->dwNumEntries > 0) {
        for (IF_INDEX i = 1; i <= ifTable->dwNumEntries; i++) {
          MIB_IFROW MibIfRow;
          MibIfRow.dwIndex = i;
          if (GetIfEntry(&MibIfRow) == NO_ERROR) {
            char name[512];
            wcstombs(name, MibIfRow.wszName, sizeof(name));
            descriptions[name + (strlen(name) - guidLen)] =
                reinterpret_cast<const char *>(MibIfRow.bDescr);
          }
        }
      }
    }
    free(ifTable);
  }
#elif defined(PLUGKIT_OS_MAC)
  {
    auto CFStringToStdString = [](CFStringRef str) -> std::string {
      char buf[2048];
      CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingUTF8);
      return buf;
    };

    CFArrayRef list = SCNetworkInterfaceCopyAll();
    CFIndex size = CFArrayGetCount(list);
    for (CFIndex i = 0; i < size; ++i) {
      SCNetworkInterfaceRef ifs =
          static_cast<SCNetworkInterfaceRef>(CFArrayGetValueAtIndex(list, i));
      const std::string &id =
          CFStringToStdString(SCNetworkInterfaceGetBSDName(ifs));
      const std::string &name =
          CFStringToStdString(SCNetworkInterfaceGetLocalizedDisplayName(ifs));
      descriptions[id] = name;
    }
    CFRelease(list);
  }
#endif

  pcap_if_t *alldevsp;
  char err[PCAP_ERRBUF_SIZE] = {'\0'};
  if (d->pcapFindalldevs(&alldevsp, err) < 0) {
    return devs;
  }

  for (pcap_if_t *ifs = alldevsp; ifs; ifs = ifs->next) {
    NetworkInterface dev;
    dev.id = ifs->name;

#if defined(PLUGKIT_OS_WIN)
    dev.name = ifs->name + (strlen(ifs->name) - guidLen);
#else
    dev.name = ifs->name;
#endif

    {
      const auto &it = descriptions.find(dev.name);
      if (it != descriptions.end()) {
        dev.name = it->second;
      }
    }

    if (ifs->description)
      dev.description = ifs->description;
    dev.loopback = ifs->flags & PCAP_IF_LOOPBACK;
    dev.link = -1;

    pcap_t *pcap = d->pcapOpenLive(ifs->name, 1600, false, 0, err);
    if (pcap) {
      dev.link = d->pcapDatalink(pcap);
      d->pcapClose(pcap);
    }

    devs.push_back(dev);
  }

  d->pcapFreealldevs(alldevsp);
  return devs;
}

bool PcapPlatform::hasPermission() const {
  if (!d->pcapLoaded)
    return false;
#if defined(PLUGKIT_OS_MAC)
  DIR *dp = opendir("/dev");
  if (dp == nullptr)
    return false;

  bool ok = true;
  struct dirent *ep;
  while ((ep = readdir(dp))) {
    std::string name(ep->d_name);
    if (name.find("bpf") == 0) {
      struct stat buf;
      if (stat(("/dev/" + name).c_str(), &buf) < 0 ||
          !(buf.st_mode & S_IRGRP) || (buf.st_gid == 0)) {
        ok = false;
        break;
      }
    }
  }

  closedir(dp);
  return ok;
#elif defined(PLUGKIT_OS_LINUX)
  char buf[2048] = {0};
  ssize_t length = readlink("/proc/self/exe", buf, sizeof(buf));
  std::string execPath(buf, length);

  cap_t cap = cap_get_proc();
  if (!cap)
    return false;

  cap_value_t cap_list[2] = {CAP_NET_ADMIN, CAP_NET_RAW};
  cap_set_flag(cap, CAP_EFFECTIVE, 2, cap_list, CAP_SET);
  cap_set_proc(cap);

  bool ok = true;
  cap_flag_value_t value;
  if (cap_get_flag(cap, CAP_NET_ADMIN, CAP_EFFECTIVE, &value) < 0 ||
      value == CAP_CLEAR)
    ok = false;

  if (cap_get_flag(cap, CAP_NET_RAW, CAP_EFFECTIVE, &value) < 0 ||
      value == CAP_CLEAR)
    ok = false;

  if (cap_get_flag(cap, CAP_NET_ADMIN, CAP_PERMITTED, &value) < 0 ||
      value == CAP_CLEAR)
    ok = false;

  if (cap_get_flag(cap, CAP_NET_RAW, CAP_PERMITTED, &value) < 0 ||
      value == CAP_CLEAR)
    ok = false;

  cap_free(cap);
  return ok;
#elif defined(PLUGKIT_OS_WIN)
  pcap_if_t *alldevsp = nullptr;
  char err[PCAP_ERRBUF_SIZE] = {'\0'};
  if (d->pcapFindalldevs(&alldevsp, err) < 0 || !alldevsp) {
    return false;
  }
  d->pcapFreealldevs(alldevsp);
#endif
  return true;
}
}
