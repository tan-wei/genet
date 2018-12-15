use genet_derive::Attr;
use genet_sdk::{cast, decoder::*, prelude::*};

struct IPv4Worker {
    layer: LayerType<IPv4>,
}

impl Worker for IPv4Worker {
    fn decode(&mut self, stack: &mut LayerStack, data: &ByteSlice) -> Result<Status> {
        let layer = Layer::new(self.layer.as_ref().clone(), data);
        stack.add_child(layer);

        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct IPv4Decoder {}

impl Decoder for IPv4Decoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(IPv4Worker {
            layer: LayerType::new("ipv4", IPv4::default()),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "ipv4".into(),
            ..Metadata::default()
        }
    }
}

def_layer_class!(IPV4_CLASS, &IPV4_ATTR);

def_attr_class!(
    IPV4_ATTR,
    "ipv4",
    child: &VERSION_ATTR,
    child: &HLEN_ATTR,
    child: &TOS_ATTR,
    child: &LENGTH_ATTR,
    child: &ID_ATTR,
    child: &FLAGS_ATTR,
    child: &FLAGS_RV_ATTR,
    child: &FLAGS_DF_ATTR,
    child: &FLAGS_MF_ATTR,
    child: &OFFSET_ATTR,
    child: &TTL_ATTR,
    child: &PROTO_ATTR,
    child: &CHECKSUM_ATTR,
    child: &SRC_ATTR,
    child: &DST_ATTR
);

def_attr_class!(VERSION_ATTR, "ipv4.version",
    cast: &cast::UInt8().map(|v| v >> 4),
    bit_range: 0 0..4
);

def_attr_class!(HLEN_ATTR, "ipv4.headerLength",
    cast: &cast::UInt8().map(|v| v & 0b00001111),
    bit_range: 0 4..8
);

def_attr_class!(TOS_ATTR, "ipv4.tos",
    cast: &cast::UInt8(),
    range: 1..2
);

def_attr_class!(LENGTH_ATTR, "ipv4.totalLength",
    cast: &cast::UInt16BE(),
    range: 2..4
);

def_attr_class!(ID_ATTR, "ipv4.id",
    cast: &cast::UInt16BE(),
    range: 4..6
);

def_attr_class!(FLAGS_ATTR, "ipv4.flags",
    cast: &cast::UInt8().map(|v| (v >> 5) & 0b00000111),
    typ: "@flags",
    bit_range: 6 0..1
);

def_attr_class!(FLAGS_RV_ATTR, "ipv4.flags.reserved",
    cast: &cast::UInt8().map(|v| v & 0b10000000 != 0),
    bit_range: 6 1..2
);

def_attr_class!(FLAGS_DF_ATTR, "ipv4.flags.dontFragment",
    cast: &cast::UInt8().map(|v| v & 0b01000000 != 0),
    bit_range: 6 2..3
);

def_attr_class!(FLAGS_MF_ATTR, "ipv4.flags.moreFragments",
    cast: &cast::UInt8().map(|v| v & 0b00100000 != 0),
    bit_range: 6 3..4
);

def_attr_class!(OFFSET_ATTR, "ipv4.fragmentOffset",
    cast: &cast::UInt16BE().map(|v| v & 0x1fff),
    bit_range: 6 4..16
);

def_attr_class!(TTL_ATTR, "ipv4.ttl",
    cast: &cast::UInt8(),
    range: 8..9
);

def_attr_class!(PROTO_ATTR, "ipv4.protocol",
    cast: &cast::UInt8(),
    typ: "@enum",
    range: 9..10
);

def_attr_class!(CHECKSUM_ATTR, "ipv4.checksum",
    cast: &cast::UInt16BE(),
    range: 10..12
);

def_attr_class!(SRC_ATTR, "ipv4.src",
    typ: "@ipv4:addr",
    alias: "_.src",
    cast: &cast::ByteSlice(),
    range: 12..16
);

def_attr_class!(DST_ATTR, "ipv4.dst",
    typ: "@ipv4:addr",
    alias: "_.dst",
    cast: &cast::ByteSlice(),
    range: 16..20
);

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

    #[genet(bit_size = 5, map = "x & 0b0001_1111")]
    fragment_offset: cast::UInt8,
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

impl From<u16> for ProtoType {
    fn from(data: u16) -> Self {
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
