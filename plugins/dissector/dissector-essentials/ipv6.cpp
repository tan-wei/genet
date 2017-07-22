#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

namespace {
int maxZeroSequence(const Slice &data) {
  int start = -1;
  int pos = -1;
  int length = 0;
  int maxLength = 0;
  for (size_t i = 0; i < data.length() / 2; ++i) {
    if (data[i * 2] == 0 && data[i * 2 + 1] == 0) {
      if (pos < 0) {
        pos = i;
      }
      ++length;
    } else {
      if (length > maxLength) {
        maxLength = length;
        start = pos;
      }
      pos = -1;
      length = 0;
    }
  }
  if (length > maxLength) {
    maxLength = length;
    start = pos;
  }
  return start;
}

std::string ipv6Addr(const Slice &data) {
  int zeroPos = maxZeroSequence(data);
  std::stringstream stream;
  stream << std::hex;
  int length = data.length() / 2;
  for (int i = 0; i < length; ++i) {
    if (zeroPos >= 0 && i >= zeroPos) {
      if (data[i * 2] == 0 && data[i * 2 + 1] == 0) {
        continue;
      } else {
        zeroPos = -1;
        stream << "::";
      }
    } else if (i > 0) {
      stream << ':';
    }
    stream << (static_cast<uint16_t>(data[i * 2] << 8) |
               static_cast<uint8_t>(data[i * 2 + 1]));
  }
  if (zeroPos >= 0) {
    stream << "::";
  }
  return stream.str();
}
}

class IPv6Dissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    Layer *analyze(Layer *layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer *child = new Layer(MNS("ipv6"));

      uint8_t header = reader.readBE<uint8_t>();
      uint8_t header2 = reader.readBE<uint8_t>();
      int version = header >> 4;
      int trafficClass =
          (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
      int flowLevel =
          reader.readBE<uint16_t>() | ((header2 & 0b00001111) << 16);

      Property *ver = new Property(MID("version"), version);
      ver->setRange(std::make_pair(0, 1));
      ver->setError(reader.lastError());
      child->addProperty(ver);

      Property *tClass = new Property(MID("tClass"), trafficClass);
      tClass->setRange(std::make_pair(0, 2));
      tClass->setError(reader.lastError());
      child->addProperty(tClass);

      Property *fLevel = new Property(MID("fLevel"), flowLevel);
      fLevel->setRange(std::make_pair(1, 4));
      fLevel->setError(reader.lastError());
      child->addProperty(fLevel);

      Property *pLen = new Property(MID("pLen"), reader.readBE<uint16_t>());
      pLen->setRange(reader.lastRange());
      pLen->setError(reader.lastError());
      child->addProperty(pLen);

      int nextHeader = reader.readBE<uint8_t>();
      auto nextHeaderRange = reader.lastRange();

      Property *nHeader = new Property(MID("nHeader"), nextHeader);
      nHeader->setRange(nextHeaderRange);
      nHeader->setError(reader.lastError());
      child->addProperty(nHeader);

      Property *hLimit = new Property(MID("hLimit"), reader.readBE<uint8_t>());
      hLimit->setRange(reader.lastRange());
      hLimit->setError(reader.lastError());
      child->addProperty(hLimit);

      const auto &srcSlice = reader.slice(16);
      Property *src = new Property(MID("src"), srcSlice);
      src->setSummary(ipv6Addr(srcSlice));
      src->setRange(reader.lastRange());
      src->setError(reader.lastError());
      child->addProperty(src);

      const auto &dstSlice = reader.slice(16);
      Property *dst = new Property(MID("dst"), dstSlice);
      dst->setSummary(ipv6Addr(dstSlice));
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
          miniid id = (nextHeader == 0) ? MID("hbyh") : MID("dst");
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

      const std::unordered_map<uint16_t, std::pair<std::string, miniid>>
          protoTable = {
              {0x01, std::make_pair("ICMP", MNS("*icmp"))},
              {0x02, std::make_pair("IGMP", MNS("*igmp"))},
              {0x06, std::make_pair("TCP", MNS("*tcp"))},
              {0x11, std::make_pair("UDP", MNS("*udp"))},
          };

      uint8_t protocolNumber = nextHeader;
      Property *proto = new Property(MID("protocol"), protocolNumber);
      const auto &type = fmt::enums(protoTable, protocolNumber,
                                    std::make_pair("Unknown", MNS("?")));
      proto->setSummary(type.first);
      if (type.second != MNS("?")) {
        child->setNs(minins(MNS("ipv6"), type.second));
      }
      proto->setRange(reader.lastRange());
      proto->setError(reader.lastError());
      child->addProperty(proto);

      child->setSummary("[" + proto->summary() + "] " + src->summary() +
                        " -> " + dst->summary());
      child->setPayload(reader.slice());
      return child;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new IPv6Dissector::Worker());
  }
  std::vector<minins> namespaces() const override {
    return std::vector<minins>{MNS("*ipv6")};
  }
};

class IPv6DissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext &ctx) const override {
    return DissectorPtr(new IPv6Dissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(
      Nan::New("factory").ToLocalChecked(),
      DissectorFactory::wrap(std::make_shared<IPv6DissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
