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
      Layer child(fmt::replace(layer->ns(), "<ipv4>", "ipv4"), "IPv4");

      uint8_t header = reader.readBE<uint8_t>();
      int version = header >> 4;
      int headerLength = header & 0b00001111;

      Property ver(PK_STRID("version"), "Version", version);
      ver.setRange(reader.lastRange());
      ver.setError(reader.lastError());

      Property hlen(PK_STRID("hLen"), "Internet Header Length", headerLength);
      hlen.setRange(reader.lastRange());
      hlen.setError(reader.lastError());

      Property tos(PK_STRID("type"), "Type Of Service", reader.readBE<uint8_t>());
      tos.setRange(reader.lastRange());
      tos.setError(reader.lastError());

      uint16_t totalLength = reader.readBE<uint16_t>();
      Property tlen(PK_STRID("tLen"), "Total Length", totalLength);
      tlen.setRange(reader.lastRange());
      tlen.setError(reader.lastError());

      Property id(PK_STRID("id"), "Identification", reader.readBE<uint16_t>());
      id.setRange(reader.lastRange());
      id.setError(reader.lastError());

      uint8_t flagAndOffset = reader.readBE<uint8_t>();
      uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

      const std::tuple<uint16_t, const char*, strid> flagTable[] = {
        std::make_tuple(0x1, "Reserved", PK_STRID("reserved") ),
        std::make_tuple(0x2, "Don\'t Fragment", PK_STRID("dontFrag") ),
        std::make_tuple(0x4, "More Fragments", PK_STRID("moreFrag") ),
      };

      Property flags(PK_STRID("flags"), "Flags", flag);
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

      uint16_t fgOffset = ((flagAndOffset & 0b00011111) << 8) | reader.readBE<uint8_t>();
      Property fragmentOffset(PK_STRID("fragmentOffset"), "Fragment Offset", fgOffset);
      fragmentOffset.setRange(std::make_pair(6, 8));
      fragmentOffset.setError(reader.lastError());

      Property ttl(PK_STRID("ttl"), "TTL", reader.readBE<uint8_t>());
      ttl.setRange(reader.lastRange());
      ttl.setError(reader.lastError());

      const std::unordered_map<
        uint16_t, std::pair<std::string,std::string>> protoTable = {
        {0x01, std::make_pair("ICMP", "icmp")},
        {0x02, std::make_pair("IGMP", "igmp")},
        {0x06, std::make_pair("TCP", "tcp")},
        {0x11, std::make_pair("UDP", "udp")},
      };

      uint8_t protocolNumber = reader.readBE<uint8_t>();
      Property proto(PK_STRID("protocol"), "Protocol", protocolNumber);
      const auto &type = fmt::enums(protoTable, protocolNumber, std::make_pair("Unknown", ""));
      proto.setSummary(type.first);
      if (!type.second.empty()) {
        child.setNs(child.ns() + " <" + type.second + ">");
      }
      proto.setRange(reader.lastRange());
      proto.setError(reader.lastError());

      Property checksum(PK_STRID("checksum"), "Header Checksum", reader.readBE<uint16_t>());
      checksum.setRange(reader.lastRange());
      checksum.setError(reader.lastError());

      const auto &srcSlice = reader.slice(4);
      Property src(PK_STRID("src"), "Source", srcSlice);
      src.setSummary(fmt::toDec(srcSlice, 1));
      src.setRange(reader.lastRange());
      src.setError(reader.lastError());

      const auto &dstSlice = reader.slice(4);
      Property dst(PK_STRID("dst"), "Destination", dstSlice);
      dst.setSummary(fmt::toDec(dstSlice, 1));
      dst.setRange(reader.lastRange());
      dst.setError(reader.lastError());

      child.setSummary("[" + proto.summary() + "] " +
        src.summary() + " -> " + dst.summary());

      child.addProperty(std::move(ver));
      child.addProperty(std::move(hlen));
      child.addProperty(std::move(tos));
      child.addProperty(std::move(tlen));
      child.addProperty(std::move(id));
      child.addProperty(std::move(flags));
      child.addProperty(std::move(fragmentOffset));
      child.addProperty(std::move(ttl));
      child.addProperty(std::move(proto));
      child.addProperty(std::move(checksum));
      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));

      child.setPayload(reader.slice(totalLength - 20));
      return std::make_shared<Layer>(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new IPv4Dissector::Worker());
  }
  std::vector<std::regex> namespaces() const override {
    return std::vector<std::regex>{std::regex("<ipv4>$")};
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
