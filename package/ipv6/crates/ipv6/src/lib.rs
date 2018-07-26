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
    fn analyze(
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
                layer.add_payload(payload, typ, "");
            }
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct IPv6Dissector {}

impl Dissector for IPv6Dissector {
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
    static ref IPV6_CLASS: LayerClass = LayerBuilder::new("ipv6")
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
    static ref VERSION_ATTR: AttrClass = AttrBuilder::new("ipv6.version")
        .decoder(decoder::UInt8().map(|v| v >> 4))
        .build();
    static ref TRAFFIC_ATTR: AttrClass = AttrBuilder::new("ipv6.trafficClass")
        .decoder(decoder::UInt16BE().map(|v| (v >> 4) & 0xff))
        .build();
    static ref FLOW_ATTR: AttrClass = AttrBuilder::new("ipv6.flowLabel")
        .decoder(
            decoder::ByteSlice()
                .map(|v| (((v[2] as u32) & 0xf) << 16) | ((v[1] as u32) << 8) | v[2] as u32)
        )
        .build();
    static ref LENGTH_ATTR: AttrClass = AttrBuilder::new("ipv6.payloadLength")
        .decoder(decoder::UInt8())
        .build();
    static ref NHEADER_ATTR: AttrClass = AttrBuilder::new("ipv6.nextHeader")
        .decoder(decoder::UInt8())
        .build();
    static ref HLIMIT_ATTR: AttrClass = AttrBuilder::new("ipv6.hopLimit")
        .decoder(decoder::UInt8())
        .build();
    static ref SRC_ATTR: AttrClass = AttrBuilder::new("ipv6.src")
        .typ("@ipv6:addr")
        .decoder(decoder::ByteSlice())
        .build();
    static ref DST_ATTR: AttrClass = AttrBuilder::new("ipv6.dst")
        .typ("@ipv6:addr")
        .decoder(decoder::ByteSlice())
        .build();
    static ref PROTOCOL_ATTR: AttrClass = AttrBuilder::new("ipv6.protocol")
        .typ("@enum")
        .decoder(decoder::UInt8())
        .build();
    static ref PROTO_MAP: HashMap<u64, (Token, AttrClass)> = hashmap!{
        0x02 => (token!("@data:igmp"), AttrBuilder::new("ipv6.protocol.igmp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x06 => (token!("@data:tcp"), AttrBuilder::new("ipv6.protocol.tcp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x11 => (token!("@data:udp"), AttrBuilder::new("ipv6.protocol.udp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x3a => (token!("@data:icmp"), AttrBuilder::new("ipv6.protocol.icmp").typ("@novalue").decoder(decoder::Const(true)).build()),
    };
}
genet_dissectors!(IPv6Dissector {});
