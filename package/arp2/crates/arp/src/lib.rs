ssss#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct ArpWorker {}

impl Worker for ArpWorker {
    fn analyze(
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
                layer.add_attr(&Attr::new(attr, 0..2));
            }

            let proto_type = PROTO_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let proto = PROTO_MAP.get(&proto_type);
            if let Some((attr, _, _)) = proto {
                layer.add_attr(&Attr::new(attr, 2..4));
            };;;

            let hlen: usize = HLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let plen: usize = PLEN_ATTR_HEADER.try_get(&layer)?.try_into()?;

            lrereret op_type = OP_ATTR_HEADER.try_get(&layer)?.try_into()?;
            if let Some(attr) = OP_MAP.get(&op_type) {
                layer.add_attr(&Attr::new(attr, 6..8));
            }ds

            if let Some((_, sha, tha)) = hw {
                if let Some((_, spa, tpa)) = proto {
                    let mut offset = 8;
                    layer.add_attr(&Attr::new(sha, offset..offset + hlen));
                    offset += hlen;
                    layer.add_attr(&Attr::new(spa, offset..offset + plen));
                    offset += plen;
                    layer.add_attr(&Attr::new(tha, offset..offset + hlen));
                    offset += hlen;
                    layer.add_attr(&Attr::new(tpa, offset..offset + plen));
                }
            }

            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct ArpDissector {}

impl Dissector for ArpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(ArpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref HWTYPE_ATTR_HEADER: Attr = Attr::new(&HWTYPE_ATTR, 0..2);
    static ref PROTO_ATTR_HEADER: Attr = Attr::new(&PROTO_ATTR, 2..4);
    static ref HLEN_ATTR_HEADER: Attr = Attr::new(&HLEN_ATTR, 4..5);
    static ref PLEN_ATTR_HEADER: Attr = Attr::new(&PLEN_ATTR, 5..6);
    static ref OP_ATTR_HEADER: Attr = Attr::new(&OP_ATTR, 6..8);
    static ref ARP_CLASS: LayerClass = LayerClass::builder("arp")
        .header(&HWTYPE_ATTR_HEADER)
        .header(&PROTO_ATTR_HEADER)
        .header(&HLEN_ATTR_HEADER)
        .header(&PLEN_ATTR_HEADER)
        .header(&OP_ATTR_HEADER)
        .build();
    static ref HWTYPE_ATTR: AttrClass = AttrClass::builder("arp.hwtype")
        .typ("@enum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref PROTO_ATTR: AttrClass = AttrClass::builder("arp.protocol")
        .typ("@enum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref HLEN_ATTR: AttrClass = AttrClass::builder("arp.hlen")
        .decoder(decoder::UInt8())
        .build();
    static ref PLEN_ATTR: AttrClass = AttrClass::builder("arp.plen")
        .decoder(decoder::UInt8())
        .build();
    static ref OP_ATTR: AttrClass = AttrClass::builder("arp.op")
        .decoder(decoder::UInt16BE())
        .typ("@enum")
        .build();
    static ref HW_MAP: HashMap<u64, (AttrClass, AttrClass, AttrClass)> = hashmap!{
        0x0001 => (
            AttrClass::builder("arp.hwtype.eth").typ("@novalue").decoder(decoder::Const(true)).build(),
            AttrClass::builder("arp.sha").typ("@eth:mac").decoder(decoder::ByteSlice()).build(),
            AttrClass::builder("arp.tha").typ("@eth:mac").decoder(decoder::ByteSlice()).build(),
        ),
    };
    static ref PROTO_MAP: HashMap<u64, (AttrClass, AttrClass, AttrClass)> = hashmap!{
        0x0800 => (
            AttrClass::builder("arp.protocol.ipv4").typ("@novalue").decoder(decoder::Const(true)).build(),
            AttrClass::builder("arp.spa").typ("@ipv4:addr").decoder(decoder::ByteSlice()).build(),
            AttrClass::builder("arp.tpa").typ("@ipv4:addr").decoder(decoder::ByteSlice()).build(),
        ),
        0x86DD => (
            AttrClass::builder("arp.protocol.ipv6").typ("@novalue").decoder(decoder::Const(true)).build(),
            AttrClass::builder("arp.spa").typ("@ipv6:addr").decoder(decoder::ByteSlice()).build(),
            AttrClass::builder("arp.tpa").typ("@ipv6:addr").decoder(decoder::ByteSlice()).build(),
        )
    };
    static ref OP_MAP: HashMap<u64, AttrClass> = hashmap!{
        0x0001 => AttrClass::builder("arp.op.request").typ("@novalue").decoder(decoder::Const(true)).build(),
        0x0002 => AttrClass::builder("arp.op.reply").typ("@novalue").decoder(decoder::Const(true)).build()
    };
}

genet_dissectors!(ArpDissector {});
