extern crate genet_sdk;

use genet_sdk::{cast, decoder::*, prelude::*};

struct IPv6Worker {}

impl Worker for IPv6Worker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        let data;

        if let Some(payload) = parent.payloads().find(|p| p.id() == token!("@data:ipv6")) {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&IPV6_CLASS, data);
        let nheader = NHEADER_ATTR_HEADER.try_get(&layer)?.try_into()?;

        loop {
            match nheader {
                // TODO:
                // case 0 | 60 # Hop-by-Hop Options, Destination Options
                // case 43  # Routing
                // case 44  # Fragment
                // case 51  # Authentication Header
                // case 50  # Encapsulating Security Payload
                // case 135 # Mobility
                // No Next Header
                59 => break,
                _ => break,
            }
        }

        let range = NHEADER_ATTR_HEADER.range();
        let proto_attr = attr!(&PROTOCOL_ATTR, range: range.clone());
        let proto = proto_attr.try_get(&layer)?.try_into()?;
        layer.add_attr(&PROTOCOL_ATTR, range.clone());
        if let Some((typ, attr)) = get_proto(proto) {
            layer.add_attr(attr, range.clone());
            let payload = layer.data().try_get(40..)?;
            layer.add_payload(Payload::new(payload, typ));
        }

        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct IPv6Decoder {}

impl Decoder for IPv6Decoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(IPv6Worker {})
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(IPV6_CLASS, "ipv6",
    alias: "_.src" "ipv6.src",
    alias: "_.dst" "ipv6.dst",
    header: &attr!(&VERSION_ATTR, bit_range: 0 0..4),
    header: &attr!(&TRAFFIC_ATTR, bit_range: 0 4..12),
    header: &attr!(&FLOW_ATTR, bit_range: 1 4..24),
    header: &attr!(&LENGTH_ATTR, range: 4..6),
    header: &NHEADER_ATTR_HEADER,
    header: &attr!(&HLIMIT_ATTR, range: 7..8),
    header: &attr!(&SRC_ATTR, range: 8..24),
    header: &attr!(&DST_ATTR, range: 24..40)
);

def_attr!(NHEADER_ATTR_HEADER,  &NHEADER_ATTR, range: 6..7);

def_attr_class!(VERSION_ATTR, "ipv6.version",
    cast: cast::UInt8().map(|v| v >> 4)
);

def_attr_class!(TRAFFIC_ATTR, "ipv6.trafficClass",
    cast: cast::UInt16BE().map(|v| (v >> 4) & 0xff)
);

def_attr_class!(FLOW_ATTR, "ipv6.flowLabel", 
    cast:
        cast::ByteSlice()
            .map(|v| (((v[2] as u32) & 0xf) << 16) | ((v[1] as u32) << 8) | v[2] as u32)
);

def_attr_class!(LENGTH_ATTR, "ipv6.payloadLength", cast: cast::UInt8());

def_attr_class!(NHEADER_ATTR, "ipv6.nextHeader", cast: cast::UInt8());

def_attr_class!(HLIMIT_ATTR, "ipv6.hopLimit", cast: cast::UInt8());

def_attr_class!(SRC_ATTR, "ipv6.src",
    typ: "@ipv6:addr",
    cast: cast::ByteSlice()
);

def_attr_class!(DST_ATTR, "ipv6.dst",
    typ: "@ipv6:addr",
    cast: cast::ByteSlice()
);

def_attr_class!(PROTOCOL_ATTR, "ipv6.protocol",
    typ: "@enum",
    cast: cast::UInt8()
);

fn get_proto(val: u64) -> Option<(Token, &'static AttrClass)> {
    match val {
        0x02 => Some((
            token!("@data:igmp"),
            attr_class_lazy!("ipv6.protocol.igmp", typ: "@novalue", value: true),
        )),
        0x06 => Some((
            token!("@data:tcp"),
            attr_class_lazy!("ipv6.protocol.tcp", typ: "@novalue", value: true),
        )),
        0x11 => Some((
            token!("@data:udp"),
            attr_class_lazy!("ipv6.protocol.udp", typ: "@novalue", value: true),
        )),
        0x3a => Some((
            token!("@data:icmp"),
            attr_class_lazy!("ipv6.protocol.icmp", typ: "@novalue", value: true),
        )),
        _ => None,
    }
}

genet_decoders!(IPv6Decoder {});
