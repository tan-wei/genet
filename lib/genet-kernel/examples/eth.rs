#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct EthWorker {}

impl Worker for EthWorker {
    fn analyze(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&ETH_CLASS, parent.data());
            let len_attr = Attr::new(&LEN_ATTR, 12..14);
            let typ_attr = Attr::new(&TYPE_ATTR, 12..14);
            let len = len_attr.try_get(&layer)?.try_into()?;
            if len <= 1500 {
                layer.add_attr(len_attr);
            } else {
                layer.add_attr(typ_attr);
            }
            if let Some(attr) = TYPE_MAP.get(&len) {
                layer.add_attr(Attr::new(attr, 12..14));
            }
            let payload = parent.data().try_get(14..)?;
            layer.add_payload(payload, "", "");
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
        .decoder(decoder::ByteSlice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.dst")
        .typ("@eth:mac")
        .decoder(decoder::ByteSlice())
        .build();
    static ref LEN_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.len")
        .decoder(decoder::UInt16BE())
        .build();
    static ref TYPE_ATTR: Ptr<AttrClass> = AttrBuilder::new("eth.type")
        .typ("@enum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref TYPE_MAP: HashMap<u64, Ptr<AttrClass>> = hashmap!{
        0x0800 => AttrBuilder::new("eth.type.ipv4").decoder(decoder::Const(true)).build(),
        0x0806 => AttrBuilder::new("eth.type.arp").decoder(decoder::Const(true)).build(),
        0x0842 => AttrBuilder::new("eth.type.wol").decoder(decoder::Const(true)).build(),
        0x86DD => AttrBuilder::new("eth.type.ipv6").decoder(decoder::Const(true)).build(),
        0x888E => AttrBuilder::new("eth.type.eap").decoder(decoder::Const(true)).build(),
    };
}

genet_dissectors!(EthDissector {});
