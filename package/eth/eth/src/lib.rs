use genet_derive::Attr;
use genet_sdk::{cast, decoder::*, prelude::*};

/// Ethernet
#[derive(Attr, Default)]
struct Eth {
    /// Source Hardware Address
    #[genet(alias = "_.src", typ = "@eth:mac", byte_size = 6)]
    src: cast::ByteSlice,

    /// Destination Hardware Address
    #[genet(alias = "_.dst", typ = "@eth:mac", byte_size = 6)]
    dst: cast::ByteSlice,

    #[genet(cond = "x <= 1500")]
    len: cast::UInt16BE,

    #[genet(cond = "x > 1500", typ = "@enum", align_before)]
    r#type: EnumNode<cast::UInt16BE, EthType>,
}

#[derive(Attr)]
enum EthType {
    IPv4,
    ARP,
    WOL,
    IPv6,
    EAP,
    Unknown,
}

impl Default for EthType {
    fn default() -> Self {
        EthType::Unknown
    }
}

impl From<u16> for EthType {
    fn from(data: u16) -> EthType {
        match data {
            0x0800 => EthType::IPv4,
            0x0806 => EthType::ARP,
            0x0842 => EthType::WOL,
            0x86DD => EthType::IPv6,
            0x888E => EthType::EAP,
            _ => EthType::Unknown,
        }
    }
}

struct EthWorker {
    layer: LayerType<Eth>,
    ipv4: WorkerBox,
    ipv6: WorkerBox,
}

impl Worker for EthWorker {
    fn decode(&mut self, stack: &mut LayerStack, data: &ByteSlice) -> Result<Status> {
        let layer = Layer::new(self.layer.as_ref().clone(), data);
        let typ = self.layer.r#type.try_get(&layer);
        stack.add_child(layer);

        let payload = data.try_get(self.layer.byte_size()..)?;
        match typ {
            Ok(EthType::IPv4) => self.ipv4.decode(stack, &payload),
            Ok(EthType::IPv6) => self.ipv6.decode(stack, &payload),
            _ => Ok(Status::Done),
        }
    }
}

#[derive(Clone)]
struct EthDecoder {}

impl Decoder for EthDecoder {
    fn new_worker(&self, ctx: &Context) -> Box<Worker> {
        Box::new(EthWorker {
            layer: LayerType::new("eth", Eth::default()),
            ipv4: ctx.decoder("ipv4").unwrap(),
            ipv6: ctx.decoder("ipv6").unwrap(),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "eth".into(),
            ..Metadata::default()
        }
    }
}

genet_decoders!(EthDecoder {});
