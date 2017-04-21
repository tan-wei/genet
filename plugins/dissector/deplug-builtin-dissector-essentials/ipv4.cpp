#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class IPv4Dissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer child("eth ipv4", "IPv4");

      uint8_t header = reader.readBE<uint8_t>();
      int version = header >> 4;
      int headerLength = header & 0b00001111;

      Property ver("version", "Version", version);
      ver.setRange(reader.lastRange());
      ver.setError(reader.lastError());

      Property hlen("headerLength", "Internet Header Length", headerLength);
      hlen.setRange(reader.lastRange());
      hlen.setError(reader.lastError());

      Property tos("type", "Typr Of Service", reader.readBE<uint8_t>());
      tos.setRange(reader.lastRange());
      tos.setError(reader.lastError());

      Property tlen("totalLength", "Total Length", reader.readBE<uint16_t>());
      tlen.setRange(reader.lastRange());
      tlen.setError(reader.lastError());

      Property id("id", "Identification", reader.readBE<uint16_t>());
      id.setRange(reader.lastRange());
      id.setError(reader.lastError());

      uint8_t flagAndOffset = reader.readBE<uint8_t>();
      uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

      static const std::tuple<uint16_t, const char*, const char*> flagTable[] = {
        {0x1, "Reserved", "reserved" },
        {0x2, "Don\'t Fragment", "dontFragment" },
        {0x4, "More Fragments", "moreFragments" },
      };

      Property flags("flags", "Flags", flag);
      std::string flagSummary;
      for (const auto& bit : flagTable) {
        bool on = std::get<0>(bit) & flag;
        Property flagBit(std::get<2>(bit), std::get<1>(bit), on);
        flagBit.setRange(reader.lastRange());
        flagBit.setError(reader.lastError());
        flags.addProperty(std::move(flagBit));
        if (on) {
          if (!flagSummary.empty()) flagSummary += ", ";
          flagSummary += std::get<1>(bit);
        }
      }
      flags.setSummary(flagSummary);
      flags.setRange(reader.lastRange());
      flags.setError(reader.lastError());

      Property fragmentOffset("fragmentOffset", "Fragment Offset", flagAndOffset & 0b11111000);
      fragmentOffset.setRange(reader.lastRange());
      fragmentOffset.setError(reader.lastError());

      child.addProperty(std::move(ver));
      child.addProperty(std::move(hlen));
      child.addProperty(std::move(tos));
      child.addProperty(std::move(tlen));
      child.addProperty(std::move(flags));
      child.addProperty(std::move(fragmentOffset));
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new IPv4Dissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("eth <ipv4>")};
  }
};

class IPv4DissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new IPv4Dissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<IPv4DissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
