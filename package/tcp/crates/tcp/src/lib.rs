#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    decoder::{self, Map},
    dissector::{Dissector, Status, Worker},
    layer::{Layer, LayerBuilder, LayerClass},
    ptr::Ptr,
    result::Result,
    slice::SliceIndex,
};
use std::collections::HashMap;

struct TcpWorker {}

impl Worker for TcpWorker {
    fn analyze(&mut self, parent: &mut Layer) -> Result<Status> {
        if let Some(payload) = parent.payloads().iter().find(|p| p.typ() == token!("tcp")) {
            let mut layer = Layer::new(&TCP_CLASS, payload.data());

            let offset_attr = Attr::new(&OFFSET_ATTR, 12..13);
            let data_offset = (offset_attr.get(&layer)?.get_u64()? * 4) as usize;
            let mut offset = 20;

            while offset < data_offset {
                let typ = layer.data()[offset];
                offset += 1;
                match typ {
                    0 => {
                        continue;
                    }
                    1 => {
                        layer.add_attr(Attr::new(&OPTIONS_NOP_ATTR, offset..offset + 1));
                        continue;
                    }
                    2 => {
                        layer.add_attr(Attr::new(&OPTIONS_MSS_ATTR, offset + 1..offset + 3));
                    }
                    3 => {
                        layer.add_attr(Attr::new(&OPTIONS_SCALE_ATTR, offset + 1..offset + 2));
                    }
                    _ => {}
                }
                let len = layer.data()[offset] as usize;
                offset += len - 1;
            }

            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct TcpDissector {}

impl Dissector for TcpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(TcpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref TCP_CLASS: Ptr<LayerClass> = LayerBuilder::new("tcp")
        .alias("_.src", "tcp.src")
        .alias("_.dst", "tcp.dst")
        .header(Attr::new(&SRC_ATTR, 0..2))
        .header(Attr::new(&DST_ATTR, 2..4))
        .header(Attr::new(&SEQ_ATTR, 4..8))
        .header(Attr::new(&ACK_ATTR, 8..12))
        .header(Attr::new(&OFFSET_ATTR, 12..13))
        .header(Attr::new(&FLAGS_ATTR, 12..14))
        .header(Attr::new(&FLAGS_NS_ATTR, 12..13))
        .header(Attr::new(&FLAGS_CWR_ATTR, 13..14))
        .header(Attr::new(&FLAGS_ECE_ATTR, 13..14))
        .header(Attr::new(&FLAGS_URG_ATTR, 13..14))
        .header(Attr::new(&FLAGS_ACK_ATTR, 13..14))
        .header(Attr::new(&FLAGS_PSH_ATTR, 13..14))
        .header(Attr::new(&FLAGS_RST_ATTR, 13..14))
        .header(Attr::new(&FLAGS_SYN_ATTR, 13..14))
        .header(Attr::new(&FLAGS_FIN_ATTR, 13..14))
        .header(Attr::new(&WINDOW_ATTR, 14..16))
        .header(Attr::new(&CHECKSUM_ATTR, 16..18))
        .header(Attr::new(&URGENT_ATTR, 18..20))
        .header(Attr::new(&OPTIONS_ATTR, 20..21))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.src")
        .typ("@tcp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.dst")
        .typ("@tcp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref SEQ_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.seq")
        .decoder(decoder::UInt32BE())
        .build();
    static ref ACK_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.ack")
        .decoder(decoder::UInt32BE())
        .build();
    static ref OFFSET_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.dataOffset")
        .decoder(decoder::UInt8().map(|v| v >> 4))
        .build();
    static ref FLAGS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags")
        .typ("@flags")
        .decoder(decoder::UInt16BE().map(|v| v & 0xfff))
        .build();
    static ref FLAGS_NS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ns")
        .decoder(decoder::UInt8().map(|v| (v & 0b0000_0001) != 0))
        .build();
    static ref FLAGS_CWR_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.cwr")
        .decoder(decoder::UInt8().map(|v| (v & 0b1000_0000) != 0))
        .build();
    static ref FLAGS_ECE_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ece")
        .decoder(decoder::UInt8().map(|v| (v & 0b0100_0000) != 0))
        .build();
    static ref FLAGS_URG_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.urg")
        .decoder(decoder::UInt8().map(|v| (v & 0b0010_0000) != 0))
        .build();
    static ref FLAGS_ACK_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ack")
        .decoder(decoder::UInt8().map(|v| (v & 0b0001_0000) != 0))
        .build();
    static ref FLAGS_PSH_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.psh")
        .decoder(decoder::UInt8().map(|v| (v & 0b0000_1000) != 0))
        .build();
    static ref FLAGS_RST_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.rst")
        .decoder(decoder::UInt8().map(|v| (v & 0b0000_0100) != 0))
        .build();
    static ref FLAGS_SYN_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.syn")
        .decoder(decoder::UInt8().map(|v| (v & 0b0000_0010) != 0))
        .build();
    static ref FLAGS_FIN_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.fin")
        .decoder(decoder::UInt8().map(|v| (v & 0b0000_0001) != 0))
        .build();
    static ref WINDOW_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.window")
        .decoder(decoder::UInt16BE())
        .build();
    static ref CHECKSUM_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.checksum")
        .decoder(decoder::UInt16BE())
        .build();
    static ref URGENT_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.urgent")
        .decoder(decoder::UInt16BE())
        .build();
    static ref OPTIONS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options")
        .typ("@nested")
        .decoder(decoder::Const(true))
        .build();
    static ref OPTIONS_NOP_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.nop")
        .decoder(decoder::Const(true))
        .build();
    static ref OPTIONS_MSS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.mss")
        .decoder(decoder::UInt16BE())
        .build();
    static ref OPTIONS_SCALE_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.scale")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_SACKP_ATTR: Ptr<AttrClass> = AttrBuilder::new(
        "tcp.options.selectiveAckPermitted"
    ).decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_SACK_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.selectiveAck")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_TS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.ts")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_TS_MY_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.ts.my")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_TS_ECHO_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.ts.echo")
        .decoder(decoder::UInt8())
        .build();
}

genet_dissectors!(TcpDissector {});
