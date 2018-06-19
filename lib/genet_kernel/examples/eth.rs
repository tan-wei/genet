#[macro_use]
extern crate genet_sdk;
extern crate genet_ffi;

use genet_sdk::{
    context::Context,
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerClass},
    result::Result,
};

struct EthWorker {}

impl Worker for EthWorker {
    fn analyze(&mut self, _: &mut Layer) -> Result<Status> {
        let class = LayerClass::new(token!("eth"));
        let layer = Layer::new(&class, &[]);
        Ok(Status::Done(vec![layer]))
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
