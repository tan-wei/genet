#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    decoder,
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerBuilder, LayerClass},
    ptr::Ptr,
    result::Result,
    slice::SliceIndex,
};
use std::collections::HashMap;

struct UdpWorker {}

impl Worker for UdpWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&UDP_CLASS, parent.data());
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
        .header(Attr::new(&SRC_ATTR, 0..6))
        .header(Attr::new(&DST_ATTR, 6..12))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.src")
        .typ("@udp:mac")
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.dst")
        .typ("@udp:mac")
        .decoder(decoder::Slice())
        .build();
    static ref LEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.length")
        .decoder(decoder::UInt8())
        .build();
    static ref CHECKSUM_ATTR: Ptr<AttrClass> = AttrBuilder::new("udp.checksum")
        .decoder(decoder::UInt8())
        .build();
}

genet_dissectors!(UdpDissector {});
