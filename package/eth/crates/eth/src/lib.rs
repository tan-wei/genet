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
    token::Token,
};
use std::collections::HashMap;

struct EthWorker {}

impl Worker for EthWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&ETH_CLASS, parent.data());
            let len_attr = Attr::new(&LEN_ATTR, 12..14);
            let typ_attr = Attr::new(&TYPE_ATTR, 12..14);
            let len = len_attr.get(&layer)?.get_u64()?;
            if len <= 1500 {
                layer.add_attr(len_attr);
            } else {
                layer.add_attr(typ_attr);
            }
            if let Some((typ, attr)) = TYPE_MAP.get(&len) {
                layer.add_attr(Attr::new(attr, 12..14));
                let payload = parent.data().get(14..)?;
                layer.add_payload(payload, typ);
            }
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct EthDissector {}

impl Dissector for EthDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(EthWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref ETH_CLASS: Ptr<LayerClass> = LayerBuilder::new("eth")
        .alias("_.src", "eth.src")
        .alias("_.dst", "eth.dst")
        .header(Attr::new(&SRC_ATTR, 0..6))
        .header(Attr::new(&DST_ATTR, 6..12))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.src")
        .typ("@eth:mac")
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.dst")
        .typ("@eth:mac")
        .decoder(decoder::Slice())
        .build();
    static ref LEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.len")
        .decoder(decoder::UInt16BE())
        .build();
    static ref TYPE_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.type")
        .typ("@enum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref TYPE_MAP: HashMap<u64, (Token, Ptr<AttrClass>)> = hashmap!{
        0x0800 => (token!("@data:ipv4"), AttrBuilder::new("eth.type.ipv4").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x0806 => (token!("@data:arp"), AttrBuilder::new("eth.type.arp").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x0842 => (token!("@data:wol"), AttrBuilder::new("eth.type.wol").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x86DD => (token!("@data:ipv6"), AttrBuilder::new("eth.type.ipv6").typ("@novalue").decoder(decoder::Const(true)).build()),
        0x888E => (token!("@data:eap"), AttrBuilder::new("eth.type.eap").typ("@novalue").decoder(decoder::Const(true)).build()),
    };
}

genet_dissectors!(EthDissector {});
