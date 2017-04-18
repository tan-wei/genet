#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/fmt.hpp>
#include <sstream>
#include <iomanip>

using namespace plugkit;

class EthernetDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      const LayerPtr& child = std::make_shared<Layer>();
      child->setNs("eth");
      child->setName("Ethernet");

      const auto &payload = layer->payload().data();
      std::stringstream dst;
      dst << std::hex << std::setfill('0');
      for (size_t i = 0; i < 6; ++i) {
        dst << ":" << std::setw(2) << static_cast<int>(
          *reinterpret_cast<const uint8_t *>(&payload[i]));
      }

      std::stringstream src;
      src << std::hex << std::setfill('0');
      for (size_t i = 6; i < 12; ++i) {
        src << ":" << std::setw(2) << static_cast<int>(
          *reinterpret_cast<const uint8_t *>(&payload[i]));
      }

      child->setSummary(src.str().substr(1) +
        " -> " +
        dst.str().substr(1) + "  " +
        std::to_string(fmt::readBE<uint64_t>(layer->payload()))
      );
      return child;
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
