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
    Layer *analyze(Layer *layer, MetaData *meta) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer *child = new Layer(MNS("udp"));

      const auto &parentSrc = layer->propertyFromId(MID("src"));
      const auto &parentDst = layer->propertyFromId(MID("dst"));

      uint16_t sourcePort = reader.readBE<uint16_t>();
      Property *src = new Property(MID("src"), sourcePort);
      src->setSummary(parentSrc->summary() + ":" + std::to_string(sourcePort));
      src->setRange(reader.lastRange());
      src->setError(reader.lastError());
      child->addProperty(src);

      uint16_t dstPort = reader.readBE<uint16_t>();
      Property *dst = new Property(MID("dst"), dstPort);
      dst->setSummary(parentDst->summary() + ":" + std::to_string(dstPort));
      dst->setRange(reader.lastRange());
      dst->setError(reader.lastError());
      child->addProperty(dst);

      uint32_t lengthNumber = reader.readBE<uint16_t>();
      Property *length = new Property(MID("length"), lengthNumber);
      length->setRange(reader.lastRange());
      length->setError(reader.lastError());
      child->addProperty(length);

      uint32_t checksumNumber = reader.readBE<uint16_t>();
      Property *checksum = new Property(MID("checksum"), checksumNumber);
      checksum->setRange(reader.lastRange());
      checksum->setError(reader.lastError());
      child->addProperty(checksum);

      child->setSummary(src->summary() + " -> " + dst->summary());
      child->setPayload(reader.slice(lengthNumber - 8));
      return child;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new UDPDissector::Worker());
  }
  std::vector<minins> namespaces() const override {
    return std::vector<minins>{MNS("*udp")};
  }
};

class UDPDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext &ctx) const override {
    return DissectorPtr(new UDPDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
               DissectorFactory::wrap(std::make_shared<UDPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
