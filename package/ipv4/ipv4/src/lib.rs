use genet_derive::Attr;
use genet_sdk::{cast, decoder::*, prelude::*};

struct IPv4Worker {
    layer: LayerType<IPv4>,
    tcp: WorkerBox,
    udp: WorkerBox,
}

impl Worker for IPv4Worker {
    fn decode(&mut self, stack: &mut LayerStack) -> Result<Status> {
        let data = stack.top().unwrap().payload();
        let mut layer = Layer::new(self.layer.as_ref().clone(), &data);

        let protocol = self.layer.protocol.try_get(&layer);
        let payload = data.try_get(self.layer.byte_size()..)?;
        layer.set_payload(&payload);
        stack.add_child(layer);

        match protocol {
            Ok(ProtoType::TCP) => self.tcp.decode(stack),
            Ok(ProtoType::UDP) => self.udp.decode(stack),
            _ => Ok(Status::Done),
        }
    }
}

#[derive(Clone)]
struct IPv4Decoder {}

impl Decoder for IPv4Decoder {
    fn new_worker(&self, ctx: &Context) -> Box<Worker> {
        Box::new(IPv4Worker {
            layer: LayerType::new("ipv4", IPv4::default()),
            tcp: ctx.decoder("tcp").unwrap(),
            udp: ctx.decoder("udp").unwrap(),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "ipv4".into(),
            ..Metadata::default()
        }
    }
}

#[derive(Attr, Default)]
struct IPv4 {
    #[genet(bit_size = 4, map = "x >> 4")]
    version: cast::UInt8,

    #[genet(bit_size = 4, map = "x & 0b00001111")]
    header_length: cast::UInt8,

    tos: cast::UInt8,

    total_length: cast::UInt16BE,

    id: cast::UInt16BE,

    #[genet(bit_size = 3, map = "(x >> 5) & 0b0000_0111", typ = "@flags")]
    flags: Node<cast::UInt8, Flags>,

    #[genet(bit_size = 13, map = "x & 0x1fff")]
    fragment_offset: cast::UInt16BE,

    ttl: cast::UInt8,

    #[genet(typ = "@enum")]
    protocol: EnumNode<cast::UInt8, ProtoType>,

    checksum: cast::UInt16BE,

    #[genet(alias = "_.src", typ = "@ipv4:addr", byte_size = 4)]
    src: cast::ByteSlice,

    #[genet(alias = "_.dst", typ = "@ipv4:addr", byte_size = 4)]
    dst: cast::ByteSlice,
}

#[derive(Attr, Default)]
struct Flags {
    reserved: cast::BitFlag,

    dont_fragment: cast::BitFlag,

    more_fragments: cast::BitFlag,
}

#[derive(Attr)]
enum ProtoType {
    ICPM,
    IGMP,
    TCP,
    UDP,
    Unknown,
}

impl Default for ProtoType {
    fn default() -> Self {
        ProtoType::Unknown
    }
}

impl From<u8> for ProtoType {
    fn from(data: u8) -> Self {
        match data {
            0x01 => ProtoType::ICPM,
            0x02 => ProtoType::IGMP,
            0x06 => ProtoType::TCP,
            0x11 => ProtoType::UDP,
            _ => Self::default(),
        }
    }
}

genet_decoders!(IPv4Decoder {});
