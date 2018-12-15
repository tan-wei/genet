use genet_sdk::{cast, decoder::*, prelude::*};

struct IPv4Worker {}

impl Worker for IPv4Worker {
    fn decode(&mut self, stack: &mut LayerStack, data: &ByteSlice) -> Result<Status> {
        let mut layer = Layer::new(&IPV4_CLASS, data);
        let proto = PROTO_ATTR.try_get(&layer)?.try_into()?;
        if let Some((typ, attr)) = get_proto(proto) {
            layer.add_attr(attr, 9..10);
            let payload = layer.data().try_get(20..)?;
            layer.add_payload(Payload::new(payload, typ));
        }

        stack.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct IPv4Decoder {}

impl Decoder for IPv4Decoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(IPv4Worker {})
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

fn get_proto(val: u64) -> Option<(Token, &'static AttrClass)> {
    match val {
        0x01 => Some((
            token!("@data:icmp"),
            attr_class_lazy!("ipv4.protocol.icmp", typ: "@novalue", value: true),
        )),
        0x02 => Some((
            token!("@data:igmp"),
            attr_class_lazy!("ipv4.protocol.igmp", typ: "@novalue", value: true),
        )),
        0x06 => Some((
            token!("@data:tcp"),
            attr_class_lazy!("ipv4.protocol.tcp", typ: "@novalue", value: true),
        )),
        0x11 => Some((
            token!("@data:udp"),
            attr_class_lazy!("ipv4.protocol.udp", typ: "@novalue", value: true),
        )),
        _ => None,
    }
}

genet_decoders!(IPv4Decoder {});
