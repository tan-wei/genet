#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class IPv6Dissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    Layer* analyze(Layer *layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer *child = new Layer(PK_STRNS("ipv6"));

      uint8_t header = reader.readBE<uint8_t>();
      uint8_t header2 = reader.readBE<uint8_t>();
      int version = header >> 4;
      int trafficClass = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
      int flowLevel = reader.readBE<uint16_t>() | ((header2 & 0b00001111) << 16);

      Property *ver = new Property(PK_STRID("version"), version);
      ver->setRange(std::make_pair(0, 1));
      ver->setError(reader.lastError());
      child->addProperty(ver);

      Property *tClass = new Property(PK_STRID("tClass"), trafficClass);
      tClass->setRange(std::make_pair(0, 2));
      tClass->setError(reader.lastError());
      child->addProperty(tClass);

      Property *fLevel = new Property(PK_STRID("fLevel"), flowLevel);
      fLevel->setRange(std::make_pair(1, 4));
      fLevel->setError(reader.lastError());
      child->addProperty(fLevel);

      Property *pLen = new Property(PK_STRID("pLen"), reader.readBE<uint16_t>());
      pLen->setRange(reader.lastRange());
      pLen->setError(reader.lastError());
      child->addProperty(pLen);

      int nextHeader = reader.readBE<uint8_t>();
      auto nextHeaderRange = reader.lastRange();

      Property *hHeader = new Property(PK_STRID("hHeader"), nextHeader);
      hHeader->setRange(nextHeaderRange);
      hHeader->setError(reader.lastError());
      child->addProperty(hHeader);

      Property *hLimit = new Property(PK_STRID("hLimit"), reader.readBE<uint8_t>());
      hLimit->setRange(reader.lastRange());
      hLimit->setError(reader.lastError());
      child->addProperty(hLimit);

      const auto &srcSlice = reader.slice(16);
      Property *src = new Property(PK_STRID("src"), srcSlice);
      src->setSummary(fmt::toHex(srcSlice, 2, 2));
      src->setRange(reader.lastRange());
      src->setError(reader.lastError());
      child->addProperty(src);

      const auto &dstSlice = reader.slice(16);
      Property *dst = new Property(PK_STRID("dst"), dstSlice);
      dst->setSummary(fmt::toHex(dstSlice, 2, 2));
      dst->setRange(reader.lastRange());
      dst->setError(reader.lastError());
      child->addProperty(dst);

      bool ext = true;
      while (ext) {
        Property *item = nullptr;
        int header = 0;
        switch (nextHeader) {
          case 0:
          case 60: // Hop-by-Hop Options, Destination Options
          {
            header = reader.readBE<uint8_t>();
            size_t extLen = reader.readBE<uint8_t>();
            size_t byteLen = (extLen + 1) * 8;
            reader.slice(byteLen);
            strid id = (nextHeader == 0) ?
              PK_STRID("hbyh") : PK_STRID("dst");
            item = new Property(id);
          }

            break;
          // TODO:
          // case 43  # Routing
          // case 44  # Fragment
          // case 51  # Authentication Header
          // case 50  # Encapsulating Security Payload
          // case 135 # Mobility
          case 59: // No Next Header
          default:
            ext = false;
            continue;
        }

        child->addProperty(item);
        nextHeader = header;
      }

      const std::unordered_map<
        uint16_t, std::pair<std::string,strid>> protoTable = {
        {0x01, std::make_pair("ICMP", PK_STRNS("*icmp"))},
        {0x02, std::make_pair("IGMP", PK_STRNS("*igmp"))},
        {0x06, std::make_pair("TCP", PK_STRNS("*tcp"))},
        {0x11, std::make_pair("UDP", PK_STRNS("*udp"))},
      };

      uint8_t protocolNumber = nextHeader;
      Property *proto = new Property(PK_STRID("protocol"), protocolNumber);
      const auto &type = fmt::enums(protoTable, protocolNumber, std::make_pair("Unknown", PK_STRNS("?")));
      proto->setSummary(type.first);
      if (!type.second.empty()) {
        child->setNs(strns(PK_STRNS("ipv6"), type.second));
      }
      proto->setRange(reader.lastRange());
      proto->setError(reader.lastError());
      child->addProperty(proto);

      child->setSummary("[" + proto->summary() + "] " +
        src->summary() + " -> " + dst->summary());
      child->setPayload(reader.slice());
      return child;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new IPv6Dissector::Worker());
  }
  std::vector<strns> namespaces() const override {
    return std::vector<strns>{PK_STRNS("*ipv6")};
  }
};

class IPv6DissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new IPv6Dissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<IPv6DissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
