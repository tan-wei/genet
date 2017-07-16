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
    Layer *analyze(Layer *layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer *child = new Layer("eth");

      const auto &srcSlice = reader.slice(6);
      Property *src = new Property(MID("src"), srcSlice);
      src->setSummary(fmt::toHex(srcSlice, 1));
      src->setRange(reader.lastRange());
      src->setError(reader.lastError());
      child->addProperty(src);

      const auto &dstSlice = reader.slice(6);
      Property *dst = new Property(MID("dst"), dstSlice);
      dst->setSummary(fmt::toHex(dstSlice, 1));
      dst->setRange(reader.lastRange());
      dst->setError(reader.lastError());
      child->addProperty(dst);

      child->setSummary(src->summary() + " -> " + dst->summary());

      auto protocolType = reader.readBE<uint16_t>();
      if (protocolType <= 1500) {
        Property *length = new Property(MID("len"), protocolType);
        length->setRange(reader.lastRange());
        length->setError(reader.lastError());
        child->addProperty(length);
      } else {
        const std::unordered_map<uint16_t, std::pair<std::string, miniid>>
            typeTable = {
                {0x0800, std::make_pair("IPv4", MNS("*ipv4"))},
                {0x0806, std::make_pair("ARP", MNS("*arp"))},
                {0x0842, std::make_pair("WoL", MNS("*wol"))},
                {0x809B, std::make_pair("AppleTalk", MNS("*aTalk"))},
                {0x80F3, std::make_pair("AARP", MNS("*aarp"))},
                {0x86DD, std::make_pair("IPv6", MNS("*ipv6"))},
            };

        Property *etherType = new Property(MID("ethType"), protocolType);
        const auto &type = fmt::enums(typeTable, protocolType,
                                      std::make_pair("Unknown", MNS("?")));
        etherType->setSummary(type.first);
        if (!type.second.empty()) {
          child->setNs(minins(MNS("eth"), type.second));
        }
        etherType->setRange(reader.lastRange());
        etherType->setError(reader.lastError());
        child->setSummary("[" + etherType->summary() + "] " + child->summary());
        child->addProperty(etherType);
      }

      child->setPayload(reader.slice());
      return child;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new EthernetDissector::Worker());
  }
  std::vector<minins> namespaces() const override {
    return std::vector<minins>{MNS("*eth")};
  }
};

class EthernetDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext &ctx) const override {
    return DissectorPtr(new EthernetDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(
      Nan::New("factory").ToLocalChecked(),
      DissectorFactory::wrap(std::make_shared<EthernetDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
