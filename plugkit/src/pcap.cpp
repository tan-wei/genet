#include "pcap_dummy.hpp"
#include "pcap_platform.hpp"
#include <cstdlib>
#include <cstring>

namespace plugkit {

Pcap::~Pcap() {}

std::unique_ptr<Pcap> Pcap::create(const SessionContext *sctx) {
  const char *pcapDummy = std::getenv("PLUGKIT_PCAP_DUMMY");
  if (pcapDummy && strlen(pcapDummy)) {
    return std::unique_ptr<Pcap>(new PcapDummy(sctx));
  } else {
    return std::unique_ptr<Pcap>(new PcapPlatform(sctx));
  }
}
} // namespace plugkit
