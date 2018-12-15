use genet_sdk::{cast, decoder::*, prelude::*};

struct IPv6Worker {}

impl Worker for IPv6Worker {
    fn decode(&mut self, stack: &mut LayerStack, _data: &ByteSlice) -> Result<Status> {
        let data;

        if let Some(payload) = stack.payloads().find(|p| p.id() == token!("@data:ipv6")) {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&IPV6_CLASS, &data);
        let nheader = NHEADER_ATTR.try_get(&layer)?.try_into()?;

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

        let range = NHEADER_ATTR.range();
        let proto = PROTOCOL_ATTR
            .try_get_range(&layer, range.clone())?
            .try_into()?;
        layer.add_attr(&PROTOCOL_ATTR, range.clone());
        if let Some((typ, attr)) = get_proto(proto) {
            layer.add_attr(attr, range.clone());
            let payload = layer.data().try_get(40..)?;
            layer.add_payload(Payload::new(payload, typ));
        }

        stack.add_child(layer);
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
            id: "ipv6".into(),
            ..Metadata::default()
        }
    }
}

def_layer_class!(IPV6_CLASS, &IPV6_ATTR);

def_attr_class!(
    IPV6_ATTR,
    "ipv6",
    child: &VERSION_ATTR,
    child: &TRAFFIC_ATTR,
    child: &FLOW_ATTR,
    child: &LENGTH_ATTR,
    child: &NHEADER_ATTR,
    child: &HLIMIT_ATTR,
    child: &SRC_ATTR,
    child: &DST_ATTR
);

def_attr_class!(VERSION_ATTR, "ipv6.version",
    cast: &cast::UInt8().map(|v| v >> 4),
    bit_range: 0 0..4
);

def_attr_class!(TRAFFIC_ATTR, "ipv6.trafficClass",
    cast: &cast::UInt16BE().map(|v| (v >> 4) & 0xff),
    bit_range: 0 4..12
);

def_attr_class!(FLOW_ATTR, "ipv6.flowLabel", 
    cast:
        &cast::ByteSlice()
            .map(|v| (((v[2] as u32) & 0xf) << 16) | ((v[1] as u32) << 8) | v[2] as u32),
    bit_range: 1 4..24
);

def_attr_class!(LENGTH_ATTR, "ipv6.payloadLength",
    cast: &cast::UInt8(),
    range: 4..6
);

def_attr_class!(NHEADER_ATTR, "ipv6.nextHeader",
    cast: &cast::UInt8(),
    range: 6..7
);

def_attr_class!(HLIMIT_ATTR, "ipv6.hopLimit",
    cast: &cast::UInt8(),
    range: 7..8
);

def_attr_class!(SRC_ATTR, "ipv6.src",
    typ: "@ipv6:addr",
    alias: "_.src",
    cast: &cast::ByteSlice(),
    range: 8..24
);

def_attr_class!(DST_ATTR, "ipv6.dst",
    typ: "@ipv6:addr",
    alias: "_.dst",
    cast: &cast::ByteSlice(),
    range: 24..40
);

def_attr_class!(PROTOCOL_ATTR, "ipv6.protocol",
    typ: "@enum",
    cast: &cast::UInt8()
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
