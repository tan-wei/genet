#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class TCPDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    Layer *analyze(Layer *layer, MetaData *meta) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer *child = new Layer(Token_get("tcp"));

      const auto &parentSrc = layer->propertyFromId(MID("src"));
      const auto &parentDst = layer->propertyFromId(MID("dst"));

      uint16_t sourcePort = reader.readBE<uint16_t>();
      Property *src = new Property(MID("src"), sourcePort);
      //       src->setSummary(parentSrc->summary() + ":" +
      //       std::to_string(sourcePort));
      src->setRange(reader.lastRange());

      child->addProperty(src);

      uint16_t dstPort = reader.readBE<uint16_t>();
      Property *dst = new Property(MID("dst"), dstPort);
      //       dst->setSummary(parentDst->summary() + ":" +
      //       std::to_string(dstPort));
      dst->setRange(reader.lastRange());

      child->addProperty(dst);

      /*
            const std::string &summary =
                (src->summary() > dst->summary())
                    ? src->summary() + " -> " + dst->summary()
                    : dst->summary() + " <- " + src->summary();
            child->setSummary(summary);
      size_t size = summary.copy(meta->streamIdentifier,
                                 sizeof(meta->streamIdentifier) - 1);

      for (size_t i = 0; i < size; ++i) {
        char &c = meta->streamIdentifier[i];
        if (c == '<' || c == '>' || c == '-')
          c = ' ';
      }
      meta->streamIdentifier[size] = '\0';
      */

      uint32_t seqNumber = reader.readBE<uint32_t>();
      Property *seq = new Property(MID("seq"), seqNumber);
      seq->setRange(reader.lastRange());

      child->addProperty(seq);

      uint32_t ackNumber = reader.readBE<uint32_t>();
      Property *ack = new Property(MID("ack"), ackNumber);
      ack->setRange(reader.lastRange());

      child->addProperty(ack);

      uint8_t offsetAndFlag = reader.readBE<uint8_t>();
      int dataOffset = offsetAndFlag >> 4;
      Property *offset = new Property(MID("dOffset"), dataOffset);
      offset->setRange(reader.lastRange());

      child->addProperty(offset);

      uint8_t flag = reader.readBE<uint8_t>() | ((offsetAndFlag & 0x1) << 8);

      const std::tuple<uint16_t, const char *, miniid> flagTable[] = {
          std::make_tuple(0x1 << 8, "NS", MID("ns")),
          std::make_tuple(0x1 << 7, "CWR", MID("cwr")),
          std::make_tuple(0x1 << 6, "ECE", MID("ece")),
          std::make_tuple(0x1 << 5, "URG", MID("urg")),
          std::make_tuple(0x1 << 4, "ACK", MID("ack")),
          std::make_tuple(0x1 << 3, "PSH", MID("psh")),
          std::make_tuple(0x1 << 2, "RST", MID("rst")),
          std::make_tuple(0x1 << 1, "SYN", MID("syn")),
          std::make_tuple(0x1 << 0, "FIN", MID("fin")),
      };

      Property *flags = new Property(MID("flags"), flag);
      std::string flagSummary;
      for (const auto &bit : flagTable) {
        bool on = std::get<0>(bit) & flag;
        Property *flagBit = new Property(std::get<2>(bit), on);
        flagBit->setRange(reader.lastRange());

        flags->addProperty(flagBit);
        if (on) {
          if (!flagSummary.empty())
            flagSummary += ", ";
          flagSummary += std::get<1>(bit);
        }
      }
      //       flags->setSummary(flagSummary);
      flags->setRange(std::make_pair(12, 14));

      child->addProperty(flags);

      Property *window = new Property(MID("window"), reader.readBE<uint16_t>());
      window->setRange(reader.lastRange());

      child->addProperty(window);

      Property *checksum =
          new Property(MID("checksum"), reader.readBE<uint16_t>());
      checksum->setRange(reader.lastRange());

      child->addProperty(checksum);

      Property *urgent = new Property(MID("urgent"), reader.readBE<uint16_t>());
      urgent->setRange(reader.lastRange());

      child->addProperty(urgent);

      Property *options = new Property(MID("options"));
      options->setRange(reader.lastRange());

      child->addProperty(options);

      size_t optionDataOffset = dataOffset * 4;
      size_t optionOffset = 20;
      while (optionDataOffset > optionOffset) {
        switch (layer->payload()[optionOffset]) {
        case 0:
          optionOffset = optionDataOffset;
          break;
        case 1: {
          Property *opt = new Property(MID("nop"));
          opt->setRange(std::make_pair(optionOffset, optionOffset + 1));

          options->addProperty(opt);
          optionOffset++;
        } break;
        case 2: {
          uint16_t size =
              fmt::readBE<uint16_t>(layer->payload(), optionOffset + 2);
          Property *opt = new Property(MID("mss"), size);
          opt->setRange(std::make_pair(optionOffset, optionOffset + 4));

          options->addProperty(opt);
          optionOffset += 4;
        } break;
        case 3: {
          uint8_t scale =
              fmt::readBE<uint8_t>(layer->payload(), optionOffset + 2);
          Property *opt = new Property(MID("scale"), scale);
          opt->setRange(std::make_pair(optionOffset, optionOffset + 2));

          options->addProperty(opt);
          optionOffset += 3;
        } break;

        case 4: {
          Property *opt = new Property(MID("ackPerm"), true);
          opt->setRange(std::make_pair(optionOffset, optionOffset + 2));

          options->addProperty(opt);
          optionOffset += 2;
        } break;

        // TODO: https://tools.ietf.org/html/rfc2018
        case 5: {
          uint8_t length =
              fmt::readBE<uint8_t>(layer->payload(), optionOffset + 1);
          Property *opt = new Property(
              MID("selAck"), layer->payload().slice(optionOffset + 2, length));
          opt->setRange(std::make_pair(optionOffset, optionOffset + length));

          options->addProperty(opt);
          optionOffset += length;
        } break;
        case 8: {
          uint32_t mt =
              fmt::readBE<uint32_t>(layer->payload(), optionOffset + 2);
          uint32_t et = fmt::readBE<uint32_t>(
              layer->payload(), optionOffset + 2 + sizeof(uint32_t));
          Property *opt = new Property(MID("ts"), std::to_string(mt) + " - " +
                                                      std::to_string(et));
          opt->setRange(std::make_pair(optionOffset, optionOffset + 10));

          Property *optmt = new Property(MID("mt"), mt);
          optmt->setRange(std::make_pair(optionOffset + 2, optionOffset + 6));

          Property *optet = new Property(MID("et"), et);
          optet->setRange(std::make_pair(optionOffset + 6, optionOffset + 01));

          opt->addProperty(optmt);
          opt->addProperty(optet);
          options->addProperty(opt);
          optionOffset += 10;
        } break;
        default:

          optionOffset = optionDataOffset;
          break;
        }
      }

      const auto &payload = layer->payload().slice(optionDataOffset);
      child->setPayload(payload);

      return child;
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new TCPDissector::Worker());
  }
  std::vector<minins> namespaces() const override {
    return std::vector<minins>{MNS("*tcp")};
  }
};

class TCPDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext &ctx) const override {
    return DissectorPtr(new TCPDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
               DissectorFactory::wrap(std::make_shared<TCPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
