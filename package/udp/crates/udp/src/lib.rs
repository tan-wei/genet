#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct UdpWorker {}

impl Worker for UdpWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.typ() == token!("@data:udp"))
        {
            let mut layer = Layer::new(&UDP_CLASS, payload.data());
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct UdpDissector {}

impl Dissector for UdpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(UdpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref UDP_CLASS: Ptr<LayerClass> = LayerBuilder::new("udp")
        .alias("_.src", "udp.src")
        .alias("_.dst", "udp.dst")
        .header(Attr::new(&SRC_ATTR, 0..2))
        .header(Attr::new(&DST_ATTR, 2..4))
        .header(Attr::new(&LEN_ATTR, 4..6))
        .header(Attr::new(&CHECKSUM_ATTR, 6..8))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.src")
        .typ("@udp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.dst")
        .typ("@udp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref LEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.length")
        .decoder(decoder::UInt16BE())
        .build();
    static ref CHECKSUM_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.checksum")
        .decoder(decoder::UInt16BE())
        .build();
}

genet_dissectors!(UdpDissector {});
