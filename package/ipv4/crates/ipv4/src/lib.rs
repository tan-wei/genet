#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    decoder::{self, Map},
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerBuilder, LayerClass},
    ptr::Ptr,
    result::Result,
    slice::SliceIndex,
    token::Token,
};
use std::collections::HashMap;

struct IPv4Worker {}

impl Worker for IPv4Worker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if let Some(payload) = parent.payloads().iter().find(|p| p.typ() == token!("ipv4")) {
            let mut layer = Layer::new(&IPV4_CLASS, payload.data());
            let proto_attr = Attr::new(&PROTO_ATTR, 9..10);
            let proto = proto_attr.get(&layer)?.get_u64()?;
            if let Some((typ, attr)) = PROTO_MAP.get(&proto) {
                layer.add_attr(Attr::new(attr, 9..10));
                let payload = layer.data().get(20..)?;
                layer.add_payload(payload, typ);
            }
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct IPv4Dissector {}

impl Dissector for IPv4Dissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(IPv4Worker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref IPV4_CLASS: Ptr<LayerClass> = LayerBuilder::new("ipv4")
        .alias("_.src", "ipv4.src")
        .alias("_.dst", "ipv4.dst")
        .header(Attr::new(&VERSION_ATTR, 0..1))
        .header(Attr::new(&HLEN_ATTR, 0..1))
        .header(Attr::new(&TOS_ATTR, 1..2))
        .header(Attr::new(&LENGTH_ATTR, 2..4))
        .header(Attr::new(&ID_ATTR, 4..6))
        .header(Attr::new(&FLAGS_ATTR, 6..7))
        .header(Attr::new(&FLAGS_RV_ATTR, 6..7))
        .header(Attr::new(&FLAGS_DF_ATTR, 6..7))
        .header(Attr::new(&FLAGS_MF_ATTR, 6..7))
        .header(Attr::new(&OFFSET_ATTR, 6..8))
        .header(Attr::new(&TTL_ATTR, 8..9))
        .header(Attr::new(&PROTO_ATTR, 9..10))
        .header(Attr::new(&CHECKSUM_ATTR, 10..12))
        .header(Attr::new(&SRC_ATTR, 12..16))
        .header(Attr::new(&DST_ATTR, 16..20))
        .build();
    static ref VERSION_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.version")
        .decoder(decoder::UInt8().map(|v| v >> 4))
        .build();
    static ref HLEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.headerLength")
        .decoder(decoder::UInt8().map(|v| v & 0b00001111))
        .build();
    static ref TOS_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.tos")
        .decoder(decoder::UInt8())
        .build();
    static ref LENGTH_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.totalLength")
        .decoder(decoder::UInt16BE())
        .build();
    static ref ID_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.id")
        .decoder(decoder::UInt16BE())
        .build();
    static ref FLAGS_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.flags")
        .decoder(decoder::UInt8().map(|v| (v >> 5) & 0b00000111))
        .typ("@flags")
        .build();
    static ref FLAGS_RV_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.flags.reserved")
        .decoder(decoder::UInt8().map(|v| v & 0b10000000 != 0))
        .build();
    static ref FLAGS_DF_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.flags.dontFragment")
        .decoder(decoder::UInt8().map(|v| v & 0b01000000 != 0))
        .build();
    static ref FLAGS_MF_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.flags.moreFragments")
        .decoder(decoder::UInt8().map(|v| v & 0b00100000 != 0))
        .build();
    static ref OFFSET_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.fragmentOffset")
        .decoder(decoder::UInt16BE().map(|v| v & 0x1fff))
        .build();
    static ref TTL_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.ttl")
        .decoder(decoder::UInt8())
        .build();
    static ref PROTO_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.protocol")
        .decoder(decoder::UInt8())
        .typ("@enum")
        .build();
    static ref CHECKSUM_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.checksum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.src")
        .typ("@ipv4:addr")
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.dst")
        .typ("@ipv4:addr")
        .decoder(decoder::Slice())
        .build();
    static ref PROTO_MAP: HashMap<u64, (Token, Ptr<AttrClass>)> = hashmap!{
        0x01 => (token!("icmp"), AttrBuilder::new("ipv4.protocol.icmp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x02 => (token!("igmp"), AttrBuilder::new("ipv4.protocol.igmp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x06 => (token!("tcp"), AttrBuilder::new("ipv4.protocol.tcp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x11 => (token!("udp"), AttrBuilder::new("ipv4.protocol.udp").typ("@novalue").decoder(decoder::Const(true)).build()),
    };
}
genet_dissectors!(IPv4Dissector {});
