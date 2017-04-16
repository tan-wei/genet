#include "pcap_platform.hpp"
#include "pcap_dummy.hpp"
#include <cstdlib>
#include <cstring>

namespace plugkit {

Pcap::~Pcap() {}

std::unique_ptr<Pcap> Pcap::create() {
  const char *pcapDummy = std::getenv("PLUGKIT_PCAP_DUMMY");
  if (pcapDummy && strlen(pcapDummy)) {
    return std::unique_ptr<Pcap>(new PcapDummy());
  } else {
    return std::unique_ptr<Pcap>(new PcapPlatform());
  }
}
}
