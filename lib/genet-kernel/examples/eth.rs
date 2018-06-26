#[macro_use]
extern crate genet_sdk;
extern crate genet_abi;

#[macro_use]
extern crate lazy_static;

use genet_sdk::{
    attr::{Attr, AttrClass},
    context::Context,
    decoder,
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerClass, LayerClassBuilder},
    ptr::Ptr,
    result::Result,
};

struct EthWorker {}

impl Worker for EthWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&ETH_CLASS, parent.data());
            let attr = Attr::new(&SRC_CLASS, 0..6);
            layer.add_attr(attr);
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
    static ref ETH_CLASS: Ptr<LayerClass> = LayerClassBuilder::new(token!("eth"))
        .alias(token!("_.src"), token!("eth.src"))
        .alias(token!("_.dst"), token!("eth.dst"))
        .header(Attr::new(&SRC_CLASS, 0..6))
        .header(Attr::new(&DST_CLASS, 6..12))
        .build();
    static ref SRC_CLASS: Ptr<AttrClass> =
        AttrClass::with_decoder(token!("eth.src"), token!("eth:mac"), decoder::Slice());
    static ref DST_CLASS: Ptr<AttrClass> =
        AttrClass::with_decoder(token!("eth.dst"), token!("eth:mac"), decoder::Slice());
}

genet_dissectors!(EthDissector {});
