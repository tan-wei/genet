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
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&ETH_CLASS, parent.data());
            let len = LEN_ATTR_HEADER.try_get(&layer)?.try_into()?;
            if len <= 1500 {
                layer.add_attr(&LEN_ATTR_HEADER);
            } else {
                layer.add_attr(&TYPE_ATTR_HEADER);
            }
            if let Some((typ, attr)) = TYPE_MAP.get(&len) {
                layer.add_attr(Attr::new(attr, 12..14));
                let payload = parent.data().try_get(14..)?;
                layer.add_payload(Payload::new(payload, typ));
            }
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct EthDecoder {}

impl Decoder for EthDecoder {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(EthWorker {}))
        } else {
            None
        }
    }
}

def_layer_class!(ETH_CLASS, "eth",
            alias: "_.src" "eth.src",
            alias: "_.dst" "eth.dst",
            header: attr!(&SRC_ATTR, 0..6),
            header: attr!(&DST_ATTR, 6..12)
        );

def_attr_class!(SRC_ATTR, "eth.src",
            typ: "@eth:mac",
            cast: cast::ByteSlice()
        );

def_attr_class!(DST_ATTR, "eth.dst",
            typ: "@eth:mac",
            cast: cast::ByteSlice()
        );

def_attr_class!(LEN_ATTR, "eth.len", cast: cast::UInt16BE());

def_attr_class!(TYPE_ATTR, "eth.type",
            typ: "@enum",
            cast: cast::UInt16BE()
        );

def_attr!(LEN_ATTR_HEADER, &LEN_ATTR, 12..14);

def_attr!(TYPE_ATTR_HEADER, &TYPE_ATTR, 12..14);

lazy_static! {
    static ref TYPE_MAP: HashMap<u64, (Token, AttrClass)> = hashmap!{
        0x0800 => (token!("@data:ipv4"), attr_class!("eth.type.ipv4", typ: "@novalue", cast: cast::Const(true))),
        0x0806 => (token!("@data:arp"), attr_class!("eth.type.arp", typ: "@novalue", cast: cast::Const(true))),
        0x0842 => (token!("@data:wol"), attr_class!("eth.type.wol", typ: "@novalue", cast: cast::Const(true))),
        0x86DD => (token!("@data:ipv6"), attr_class!("eth.type.ipv6", typ: "@novalue", cast: cast::Const(true))),
        0x888E => (token!("@data:eap"), attr_class!("eth.type.eap", typ: "@novalue", cast: cast::Const(true))),
    };
}

genet_decoders!(EthDecoder {});
