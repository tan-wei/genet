#[macro_use]
extern crate genet_sdk;
extern crate genet_abi;

use genet_sdk::{
    context::Context,
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerClass},
    result::Result,
};

struct EthWorker {}

impl Worker for EthWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let class = LayerClass::new(token!("eth"));
            let layer = Layer::new(&class, &[]);
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

genet_dissectors!(EthDissector {});
