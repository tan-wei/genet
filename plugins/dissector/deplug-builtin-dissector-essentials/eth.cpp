#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class EthernetDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer child("eth");

      const auto &srcSlice = reader.slice(6);
      Property src(PK_STRID("src"), "Source", srcSlice);
      src.setSummary(fmt::toHex(srcSlice, 1));
      src.setRange(reader.lastRange());
      src.setError(reader.lastError());

      const auto &dstSlice = reader.slice(6);
      Property dst(PK_STRID("dst"), "Destination", dstSlice);
      dst.setSummary(fmt::toHex(dstSlice, 1));
      dst.setRange(reader.lastRange());
      dst.setError(reader.lastError());

      child.setSummary(src.summary() + " -> " + dst.summary());
      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));

      auto protocolType = reader.readBE<uint16_t>();
      if (protocolType <= 1500) {
        Property length(PK_STRID("len"), "Length", protocolType);
        length.setRange(reader.lastRange());
        length.setError(reader.lastError());
        child.addProperty(std::move(length));
      } else {
        const std::unordered_map<
          uint16_t, std::pair<std::string,std::string>> typeTable = {
          {0x0800, std::make_pair("IPv4", "ipv4")},
          {0x0806, std::make_pair("ARP", "arp")},
          {0x0842, std::make_pair("WoL", "wol")},
          {0x809B, std::make_pair("AppleTalk", "appleTalk")},
          {0x80F3, std::make_pair("AARP", "aarp")},
          {0x86DD, std::make_pair("IPv6", "ipv6")},
        };

        Property etherType(PK_STRID("ethType"), "EtherType", protocolType);
        const auto &type = fmt::enums(typeTable, protocolType, std::make_pair("Unknown", ""));
        etherType.setSummary(type.first);
        if (!type.second.empty()) {
          child.setNs(child.ns() + " <" + type.second + ">");
        }
        etherType.setRange(reader.lastRange());
        etherType.setError(reader.lastError());
        child.setSummary("[" + etherType.summary() + "] " + child.summary());
        child.addProperty(std::move(etherType));
      }

      child.setPayload(reader.slice());
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new EthernetDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<eth>$")};
  }
};

class EthernetDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new EthernetDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<EthernetDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
