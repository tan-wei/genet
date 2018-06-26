#[macro_use]
extern crate genet_sdk;
extern crate genet_abi;

#[macro_use]
extern crate lazy_static;

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    decoder,
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerBuilder, LayerClass},
    ptr::Ptr,
    result::Result,
};

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
            layer.add_payload(&parent.data()[14..], token!());
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
    static ref ETH_CLASS: Ptr<LayerClass> = LayerBuilder::new(token!("eth"))
        .alias(token!("_.src"), token!("eth.src"))
        .alias(token!("_.dst"), token!("eth.dst"))
        .header(Attr::new(&SRC_ATTR, 0..6))
        .header(Attr::new(&DST_ATTR, 6..12))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new(token!("eth.src"))
        .typ(token!("eth:mac"))
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new(token!("eth.dst"))
        .typ(token!("eth:mac"))
        .decoder(decoder::Slice())
        .build();
    static ref LEN_ATTR: Ptr<AttrClass> = AttrBuilder::new(token!("eth.len"))
        .decoder(decoder::UInt16BE())
        .build();
    static ref TYPE_ATTR: Ptr<AttrClass> = AttrBuilder::new(token!("eth.type"))
        .decoder(decoder::UInt16BE())
        .build();
}

genet_dissectors!(EthDissector {});
