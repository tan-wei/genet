#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class UDPDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    LayerPtr analyze(const LayerConstPtr &layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer child(fmt::replace(layer->ns(), "<udp>", "udp"), "UDP");

      const auto& parentSrc = layer->propertyFromId("src");
      const auto& parentDst = layer->propertyFromId("dst");

      uint16_t sourcePort = reader.readBE<uint16_t>();
      Property src("src", "Source", sourcePort);
      src.setSummary(parentSrc->summary() + ":" + std::to_string(sourcePort));
      src.setRange(reader.lastRange());
      src.setError(reader.lastError());

      uint16_t dstPort = reader.readBE<uint16_t>();
      Property dst("dst", "Destination", dstPort);
      dst.setSummary(parentDst->summary() + ":" + std::to_string(dstPort));
      dst.setRange(reader.lastRange());
      dst.setError(reader.lastError());

      uint32_t lengthNumber = reader.readBE<uint16_t>();
      Property length("length", "Length", lengthNumber);
      length.setRange(reader.lastRange());
      length.setError(reader.lastError());

      uint32_t checksumNumber = reader.readBE<uint16_t>();
      Property checksum("checksum", "Checksum", checksumNumber);
      checksum.setRange(reader.lastRange());
      checksum.setError(reader.lastError());

      child.setSummary(src.summary() + " -> " + dst.summary());

      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));
      child.addProperty(std::move(length));
      child.addProperty(std::move(checksum));
      child.setPayload(reader.slice(lengthNumber - 8));
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new UDPDissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<udp>$")};
  }
};

class UDPDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new UDPDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<UDPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
