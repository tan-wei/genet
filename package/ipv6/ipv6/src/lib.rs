#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct IPv6Worker {}

impl Worker for IPv6Worker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:ipv6"))
        {
            let mut layer = Layer::new(&IPV6_CLASS, payload.data());
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
            let proto_attr = Attr::new(&PROTOCOL_ATTR, range.clone());
            let proto = proto_attr.try_get(&layer)?.try_into()?;
            layer.add_attr(proto_attr);
            if let Some((typ, attr)) = PROTO_MAP.get(&proto) {
                layer.add_attr(Attr::new(attr, range.clone()));
                let payload = layer.data().try_get(40..)?;
                layer.add_payload(Payload::new(payload, typ));
            }
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct IPv6Decoder {}

impl Decoder for IPv6Decoder {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(IPv6Worker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref NHEADER_ATTR_HEADER: Attr = Attr::new(&NHEADER_ATTR, 6..7);
    static ref IPV6_CLASS: LayerClass = LayerClass::builder("ipv6")
        .alias("_.src", "ipv6.src")
        .alias("_.dst", "ipv6.dst")
        .header(Attr::new(&VERSION_ATTR, 0..1))
        .header(Attr::new(&TRAFFIC_ATTR, 0..2))
        .header(Attr::new(&FLOW_ATTR, 1..4))
        .header(Attr::new(&LENGTH_ATTR, 4..6))
        .header(&NHEADER_ATTR_HEADER)
        .header(Attr::new(&HLIMIT_ATTR, 7..8))
        .header(Attr::new(&SRC_ATTR, 8..24))
        .header(Attr::new(&DST_ATTR, 24..40))
        .build();
    static ref VERSION_ATTR: AttrClass = AttrClass::builder("ipv6.version")
        .cast(cast::UInt8().map(|v| v >> 4))
        .build();
    static ref TRAFFIC_ATTR: AttrClass = AttrClass::builder("ipv6.trafficClass")
        .cast(cast::UInt16BE().map(|v| (v >> 4) & 0xff))
        .build();
    static ref FLOW_ATTR: AttrClass = AttrClass::builder("ipv6.flowLabel")
        .cast(
            cast::ByteSlice()
                .map(|v| (((v[2] as u32) & 0xf) << 16) | ((v[1] as u32) << 8) | v[2] as u32)
        ).build();
    static ref LENGTH_ATTR: AttrClass = AttrClass::builder("ipv6.payloadLength")
        .cast(cast::UInt8())
        .build();
    static ref NHEADER_ATTR: AttrClass = AttrClass::builder("ipv6.nextHeader")
        .cast(cast::UInt8())
        .build();
    static ref HLIMIT_ATTR: AttrClass = AttrClass::builder("ipv6.hopLimit")
        .cast(cast::UInt8())
        .build();
    static ref SRC_ATTR: AttrClass = AttrClass::builder("ipv6.src")
        .typ("@ipv6:addr")
        .cast(cast::ByteSlice())
        .build();
    static ref DST_ATTR: AttrClass = AttrClass::builder("ipv6.dst")
        .typ("@ipv6:addr")
        .cast(cast::ByteSlice())
        .build();
    static ref PROTOCOL_ATTR: AttrClass = AttrClass::builder("ipv6.protocol")
        .typ("@enum")
        .cast(cast::UInt8())
        .build();
    static ref PROTO_MAP: HashMap<u64, (Token, AttrClass)> = hashmap!{
        0x02 => (token!("@data:igmp"), AttrClass::builder("ipv6.protocol.igmp").typ("@novalue").cast(cast::Const(true)).build()),
        0x06 => (token!("@data:tcp"), AttrClass::builder("ipv6.protocol.tcp").typ("@novalue").cast(cast::Const(true)).build()),
        0x11 => (token!("@data:udp"), AttrClass::builder("ipv6.protocol.udp").typ("@novalue").cast(cast::Const(true)).build()),
        0x3a => (token!("@data:icmp"), AttrClass::builder("ipv6.protocol.icmp").typ("@novalue").cast(cast::Const(true)).build()),
    };
}
genet_decoders!(IPv6Decoder {});
