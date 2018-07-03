#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    decoder,
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
        if parent.id() == token!("[link-1]") {
            let mut layer = Layer::new(&TCP_CLASS, parent.data());
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
        .header(Attr::new(&SRC_ATTR, 0..6))
        .header(Attr::new(&DST_ATTR, 6..12))
        .build();
    static ref SRC_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.src")
        .typ("@tcp:mac")
        .decoder(decoder::Slice())
        .build();
    static ref DST_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.dst")
        .typ("@tcp:mac")
        .decoder(decoder::Slice())
        .build();
    static ref SEQ_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.seq")
        .decoder(decoder::UInt8())
        .build();
    static ref ACK_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.ack")
        .decoder(decoder::UInt8())
        .build();
    static ref OFFSET_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.dataOffset")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_NS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ns")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_CWR_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.cwr")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_ECE_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ece")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_URG_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.urg")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_ACK_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.ack")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_PSH_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.psh")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_RST_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.rst")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_SYN_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.syn")
        .decoder(decoder::UInt8())
        .build();
    static ref FLAGS_FIN_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.flags.fin")
        .decoder(decoder::UInt8())
        .build();
    static ref WINDOW_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.window")
        .decoder(decoder::UInt8())
        .build();
    static ref CHECKSUM_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.checksum")
        .decoder(decoder::UInt8())
        .build();
    static ref URGENT_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.urgent")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_NOP_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.nop")
        .decoder(decoder::UInt8())
        .build();
    static ref OPTIONS_MSS_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.options.mss")
        .decoder(decoder::UInt8())
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
