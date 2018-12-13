extern crate genet_sdk;

use genet_sdk::{cast, decoder::*, prelude::*};

struct ArpWorker {}

impl Worker for ArpWorker {
    fn decode(&mut self, parent: &mut Parent) -> Result<Status> {
        let data;

        if let Some(payload) = parent.payloads().find(|p| p.id() == token!("@data:arp")) {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&ARP_CLASS, data);

        let hw_type = HWTYPE_ATTR.try_get(&layer)?.try_into()?;
        let hw = get_hw(hw_type);
        if let Some((attr, _, _)) = hw {
            layer.add_attr(attr, 0..2);
        }

        let proto_type = PROTO_ATTR.try_get(&layer)?.try_into()?;
        let proto = get_proto(proto_type);
        if let Some((attr, _, _)) = proto {
            layer.add_attr(attr, 2..4);
        }

        let hlen: usize = HLEN_ATTR.try_get(&layer)?.try_into()?;
        let plen: usize = PLEN_ATTR.try_get(&layer)?.try_into()?;

        let op_type = OP_ATTR.try_get(&layer)?.try_into()?;
        if let Some(attr) = get_op(op_type) {
            layer.add_attr(attr, 6..8);
        }

        if let Some((_, sha, tha)) = hw {
            if let Some((_, spa, tpa)) = proto {
                let mut offset = 8;
                layer.add_attr(sha, offset..offset + hlen);
                offset += hlen;
                layer.add_attr(spa, offset..offset + plen);
                offset += plen;
                layer.add_attr(tha, offset..offset + hlen);
                offset += hlen;
                layer.add_attr(tpa, offset..offset + plen);
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
            id: "arp".into(),
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(ARP_CLASS, &ARP_ATTR);

def_attr_class!(
    ARP_ATTR,
    "arp",
    child: &HWTYPE_ATTR,
    child: &PROTO_ATTR,
    child: &HLEN_ATTR,
    child: &PLEN_ATTR,
    child: &OP_ATTR
);

def_attr_class!(HWTYPE_ATTR, "arp.hwtype",
    typ: "@enum",
    cast: &cast::UInt16BE(),
    range: 0..2
);

def_attr_class!(PROTO_ATTR, "arp.protocol",
    typ: "@enum",
    cast: &cast::UInt16BE(),
    range: 2..4
);

def_attr_class!(HLEN_ATTR, "arp.hlen",
    cast: &cast::UInt8(),
    range: 4..5
);

def_attr_class!(PLEN_ATTR, "arp.plen",
    cast: &cast::UInt8(),
    range: 5..6
);

def_attr_class!(OP_ATTR, "arp.op",
    cast: &cast::UInt16BE(),
    typ: "@enum",
    range: 6..8
);

fn get_hw(val: u64) -> Option<(&'static AttrClass, &'static AttrClass, &'static AttrClass)> {
    match val {
        0x0001 => Some((
            attr_class_lazy!("arp.hwtype.eth", typ: "@novalue", value: true),
            attr_class_lazy!("arp.sha", typ: "@eth:mac", alias: "_.src", cast: &cast::ByteSlice()),
            attr_class_lazy!("arp.tha", typ: "@eth:mac", alias: "_.dst", cast: &cast::ByteSlice()),
        )),
        _ => None,
    }
}

fn get_proto(val: u64) -> Option<(&'static AttrClass, &'static AttrClass, &'static AttrClass)> {
    match val {
        0x0800 => Some((
            attr_class_lazy!("arp.protocol.ipv4", typ: "@novalue", value: true),
            attr_class_lazy!("arp.spa", typ: "@ipv4:addr", cast: &cast::ByteSlice()),
            attr_class_lazy!("arp.tpa", typ: "@ipv4:addr", cast: &cast::ByteSlice()),
        )),
        0x86DD => Some((
            attr_class_lazy!("arp.protocol.ipv6", typ: "@novalue", value: true),
            attr_class_lazy!("arp.spa", typ: "@ipv6:addr", cast: &cast::ByteSlice()),
            attr_class_lazy!("arp.tpa", typ: "@ipv6:addr", cast: &cast::ByteSlice()),
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
