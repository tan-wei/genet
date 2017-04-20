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
      Layer child("eth", "Ethernet");

      const auto &srcSlice = reader.slice(6);
      Property src("src", "Source", srcSlice);
      src.setSummary(fmt::toHex(srcSlice, 1));
      src.setRange(reader.lastRange());
      src.setError(reader.lastError());

      const auto &dstSlice = reader.slice(6);
      Property dst("dst", "Destination", dstSlice);
      dst.setSummary(fmt::toHex(dstSlice, 1));
      dst.setRange(reader.lastRange());
      dst.setError(reader.lastError());

      child.setSummary(src.summary() + " -> " + dst.summary());
      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));

      auto protocolType = reader.readBE<uint16_t>();
      if (protocolType <= 1500) {
        Property length("len", "Length", protocolType);
        length.setRange(reader.lastRange());
        length.setError(reader.lastError());
        child.addProperty(std::move(length));
      } else {
        static const std::unordered_map<uint16_t, std::string> types = {
          {0x0800, "IPv4"},
          {0x0806, "ARP"},
          {0x0842, "WoL"},
          {0x809B, "AppleTalk"},
          {0x80F3, "AARP"},
          {0x86DD, "IPv6"},
        };

        Property etherType("etherType", "EtherType", protocolType);
        etherType.setSummary(fmt::enums(types, protocolType, "Unknown"));
        etherType.setRange(reader.lastRange());
        etherType.setError(reader.lastError());
        child.setSummary("[" + etherType.summary() + "] " + child.summary());
        child.addProperty(std::move(etherType));
      }

      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new EthernetDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<eth>")};
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
