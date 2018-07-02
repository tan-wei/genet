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
};
use std::collections::HashMap;

struct IPv4Worker {}

impl Worker for IPv4Worker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("eth") {
            let mut layer = Layer::new(&IPV4_CLASS, parent.data().get(14..)?);
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
        .header(Attr::new(&SRC_ATTR, 0..6))
        .header(Attr::new(&DST_ATTR, 6..12))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.src")
        .typ("@ipv4:addr")
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.dst")
        .typ("@ipv4:addr")
        .decoder(decoder::Slice())
        .build();
    static ref VERSION_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.version")
        .decoder(decoder::UInt8().map(|v| v >> 4))
        .build();
    static ref HLEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("ipv4.headerLength")
        .decoder(decoder::UInt8().map(|v| v & 0b00001111))
        .build();
}

genet_dissectors!(IPv4Dissector {});
