#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <sstream>
#include <iomanip>

using namespace plugkit;

class EthernetDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      Layer child("eth", "Ethernet");

      const auto &payload = layer->payload();
      const auto &srcSlice = payload.slice(0, 6);
      Property src("src", "Source", srcSlice);
      src.setSummary(fmt::toHex(srcSlice, 1));
      src.setRange(fmt::range(payload, srcSlice));

      const auto &dstSlice = payload.slice(6, 6);
      Property dst("dst", "Destination", dstSlice);
      dst.setSummary(fmt::toHex(dstSlice, 1));
      dst.setRange(fmt::range(payload, dstSlice));

      child.setSummary(src.summary() + " -> " + dst.summary());
      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));

      auto protocolType = fmt::readBE<uint16_t>(payload);
      if (protocolType <= 1500) {

      } else {
        Property etherType("etherType", "EtherType", static_cast<double>(protocolType));
        etherType.setSummary(std::to_string(protocolType));
        etherType.setRange("12:14");
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
