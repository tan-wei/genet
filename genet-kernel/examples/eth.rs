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
}

impl Worker for EthWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
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
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

genet_decoders!(EthDecoder {});
