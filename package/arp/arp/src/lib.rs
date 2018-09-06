#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct ArpWorker {}

impl Worker for ArpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:arp"))
        {
            let mut layer = Layer::new(&ARP_CLASS, payload.data());

            let hw_type = HWTYPE_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let hw = HW_MAP.get(&hw_type);
            if let Some((attr, _, _)) = hw {
                layer.add_attr(attr!(attr, 0..2));
            }

            let proto_type = PROTO_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let proto = PROTO_MAP.get(&proto_type);
            if let Some((attr, _, _)) = proto {
                layer.add_attr(attr!(attr, 2..4));
            }

            let hlen: usize = HLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let plen: usize = PLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;

            let op_type = OP_ATTR_HEADER.try_get(&layer)?.try_into()?;
            if let Some(attr) = OP_MAP.get(&op_type) {
                layer.add_attr(attr!(attr, 6..8));
            }

            if let Some((_, sha, tha)) = hw {
                if let Some((_, spa, tpa)) = proto {
                    let mut offset = 8;
                    layer.add_attr(attr!(sha, offset..offset + hlen));
                    offset += hlen;
                    layer.add_attr(attr!(spa, offset..offset + plen));
                    offset += plen;
                    layer.add_attr(attr!(tha, offset..offset + hlen));
                    offset += hlen;
                    layer.add_attr(attr!(tpa, offset..offset + plen));
                }
            }

            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct ArpDecoder {}

impl Decoder for ArpDecoder {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(ArpWorker {}))
        } else {
            None
        }
    }
}

def_layer_class!(ARP_CLASS, "arp",
    alias: "_.src" "arp.sha",
    alias: "_.dst" "arp.tha",
    header: &HWTYPE_ATTR_HEADER,
    header: &PROTO_ATTR_HEADER,
    header: &HLEN_ATTR_HEADER,
    header: &PLEN_ATTR_HEADER,
    header: &OP_ATTR_HEADER
);

def_attr!(HWTYPE_ATTR_HEADER, &HWTYPE_ATTR, 0..2);
def_attr!(PROTO_ATTR_HEADER, &PROTO_ATTR, 2..4);
def_attr!(HLEN_ATTR_HEADER, &HLEN_ATTR, 4..5);
def_attr!(PLEN_ATTR_HEADER, &PLEN_ATTR, 5..6);
def_attr!(OP_ATTR_HEADER, &OP_ATTR, 6..8);

def_attr_class!(HWTYPE_ATTR, "arp.hwtype",
    typ: "@enum",
    cast: cast::UInt16BE()
);

def_attr_class!(PROTO_ATTR, "arp.protocol",
    typ: "@enum",
    cast: cast::UInt16BE()
);

def_attr_class!(HLEN_ATTR, "arp.hlen", cast: cast::UInt8());

def_attr_class!(PLEN_ATTR, "arp.plen", cast: cast::UInt8());

def_attr_class!(OP_ATTR, "arp.op",
    cast: cast::UInt16BE(),
    typ: "@enum"
);

lazy_static! {
    static ref HW_MAP: HashMap<u64, (AttrClass, AttrClass, AttrClass)> = hashmap!{
        0x0001 => (
            attr_class!("arp.hwtype.eth", typ: "@novalue", cast: cast::Const(true)),
            attr_class!("arp.sha", typ: "@eth:mac", cast: cast::ByteSlice()),
            attr_class!("arp.tha", typ: "@eth:mac", cast: cast::ByteSlice()),
        ),
    };
    static ref PROTO_MAP: HashMap<u64, (AttrClass, AttrClass, AttrClass)> = hashmap!{
        0x0800 => (
            attr_class!("arp.protocol.ipv4", typ: "@novalue", cast: cast::Const(true)),
            attr_class!("arp.spa", typ: "@ipv4:addr", cast: cast::ByteSlice()),
            attr_class!("arp.tpa", typ: "@ipv4:addr", cast: cast::ByteSlice()),
        ),
        0x86DD => (
            attr_class!("arp.protocol.ipv6", typ: "@novalue", cast: cast::Const(true)),
            attr_class!("arp.spa", typ: "@ipv6:addr", cast: cast::ByteSlice()),
            attr_class!("arp.tpa", typ: "@ipv6:addr", cast: cast::ByteSlice()),
        )
    };
    static ref OP_MAP: HashMap<u64, AttrClass> = hashmap!{
        0x0001 => attr_class!("arp.op.request", typ: "@novalue", cast: cast::Const(true)),
        0x0002 => attr_class!("arp.op.reply", typ: "@novalue", cast: cast::Const(true))
    };
}

genet_decoders!(ArpDecoder {});
