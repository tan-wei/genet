#[macro_use]
extern crate genet_sdk;

use genet_sdk::{cast, decoder::*, prelude::*};

struct ArpWorker {}

impl Worker for ArpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        let data;

        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:arp"))
        {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&ARP_CLASS, data);

        let hw_type = HWTYPE_ATTR_HEADER.try_get(&layer)?.try_into()?;
        let hw = get_hw(hw_type);
        if let Some((attr, _, _)) = hw {
            layer.add_attr(attr!(attr, range: 0..2));
        }

        let proto_type = PROTO_ATTR_HEADER.try_get(&layer)?.try_into()?;
        let proto = get_proto(proto_type);
        if let Some((attr, _, _)) = proto {
            layer.add_attr(attr!(attr, range: 2..4));
        }

        let hlen: usize = HLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;
        let plen: usize = PLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;

        let op_type = OP_ATTR_HEADER.try_get(&layer)?.try_into()?;
        if let Some(attr) = get_op(op_type) {
            layer.add_attr(attr!(attr, range: 6..8));
        }

        if let Some((_, sha, tha)) = hw {
            if let Some((_, spa, tpa)) = proto {
                let mut offset = 8;
                layer.add_attr(attr!(sha, range: offset..offset + hlen));
                offset += hlen;
                layer.add_attr(attr!(spa, range: offset..offset + plen));
                offset += plen;
                layer.add_attr(attr!(tha, range: offset..offset + hlen));
                offset += hlen;
                layer.add_attr(attr!(tpa, range: offset..offset + plen));
            }
        }

        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct ArpDecoder {}

impl Decoder for ArpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(ArpWorker {})
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
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

def_attr!(HWTYPE_ATTR_HEADER,  &HWTYPE_ATTR, range: 0..2);
def_attr!(PROTO_ATTR_HEADER,  &PROTO_ATTR, range: 2..4);
def_attr!(HLEN_ATTR_HEADER,  &HLEN_ATTR, range: 4..5);
def_attr!(PLEN_ATTR_HEADER,  &PLEN_ATTR, range: 5..6);
def_attr!(OP_ATTR_HEADER,  &OP_ATTR, range: 6..8);

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

fn get_hw(val: u64) -> Option<(&'static AttrClass, &'static AttrClass, &'static AttrClass)> {
    match val {
        0x0001 => Some((
            attr_class_lazy!("arp.hwtype.eth", typ: "@novalue", value: true),
            attr_class_lazy!("arp.sha", typ: "@eth:mac", cast: cast::ByteSlice()),
            attr_class_lazy!("arp.tha", typ: "@eth:mac", cast: cast::ByteSlice()),
        )),
        _ => None,
    }
}

fn get_proto(val: u64) -> Option<(&'static AttrClass, &'static AttrClass, &'static AttrClass)> {
    match val {
        0x0800 => Some((
            attr_class_lazy!("arp.protocol.ipv4", typ: "@novalue", value: true),
            attr_class_lazy!("arp.spa", typ: "@ipv4:addr", cast: cast::ByteSlice()),
            attr_class_lazy!("arp.tpa", typ: "@ipv4:addr", cast: cast::ByteSlice()),
        )),
        0x86DD => Some((
            attr_class_lazy!("arp.protocol.ipv6", typ: "@novalue", value: true),
            attr_class_lazy!("arp.spa", typ: "@ipv6:addr", cast: cast::ByteSlice()),
            attr_class_lazy!("arp.tpa", typ: "@ipv6:addr", cast: cast::ByteSlice()),
        )),
        _ => None,
    }
}

fn get_op(val: u64) -> Option<&'static AttrClass> {
    match val {
        0x0001 => Some(attr_class_lazy!("arp.op.request", typ: "@novalue", value: true)),
        0x0002 => Some(attr_class_lazy!("arp.op.reply", typ: "@novalue", value: true)),
        _ => None,
    }
}

genet_decoders!(ArpDecoder {});
