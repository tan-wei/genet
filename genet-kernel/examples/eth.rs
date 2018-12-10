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
    r#type: EnumNode<cast::UInt16BE, EthTypeEnum>,
}

#[derive(Attr)]
enum EthTypeEnum {
    IPv4,
    ARP,
    WOL,
    IPv6,
    EAP,
    Unknown,
}

impl Default for EthTypeEnum {
    fn default() -> Self {
        EthTypeEnum::Unknown
    }
}

impl From<u16> for EthTypeEnum {
    fn from(data: u16) -> EthTypeEnum {
        match data {
            0x0800 => EthTypeEnum::IPv4,
            0x0806 => EthTypeEnum::ARP,
            0x0842 => EthTypeEnum::WOL,
            0x86DD => EthTypeEnum::IPv6,
            0x888E => EthTypeEnum::EAP,
            _ => EthTypeEnum::Unknown,
        }
    }
}

struct EthWorker {
    layer: LayerType<Eth>,
}

impl Worker for EthWorker {
    fn decode(&mut self, _stack: &LayerStack, parent: &mut Parent) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let data = parent.payloads().next().unwrap().data();
            let layer = Layer::new(self.layer.as_ref().clone(), data);
            parent.add_child(layer);
            Ok(Status::Done)
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct EthDecoder {}

impl Decoder for EthDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(EthWorker {
            layer: LayerType::new("eth", Eth::default()),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "eth".into(),
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

genet_decoders!(EthDecoder {});
