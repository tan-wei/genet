#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

use genet_sdk::prelude::*;

struct TcpWorker {}

impl Worker for TcpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:tcp"))
        {
            let mut layer = Layer::new(&TCP_CLASS, payload.data());

            let data_offset: usize = OFFSET_ATTR_HEADER.try_get(&layer)?.try_into()?;
            let data_offset = data_offset * 4;
            let mut offset = 20;

            while offset < data_offset {
                let typ = layer.data().try_get(offset)?;
                let len = layer.data().try_get(offset + 1)? as usize;
                match typ {
                    0 => {
                        offset += 1;
                        continue;
                    }
                    1 => {
                        layer.add_attr(Attr::new(&OPTIONS_NOP_ATTR, offset..offset + 1));
                        offset += 1;
                        continue;
                    }
                    2 => {
                        layer.add_attr(Attr::new(&OPTIONS_MSS_ATTR, offset..offset + len));
                    }
                    3 => {
                        layer.add_attr(Attr::new(&OPTIONS_SCALE_ATTR, offset..offset + len));
                    }
                    4 => {
                        layer.add_attr(Attr::new(&OPTIONS_SACKP_ATTR, offset..offset + len));
                    }
                    5 => {
                        layer.add_attr(Attr::new(&OPTIONS_SACK_ATTR, offset..offset + len));
                    }
                    8 => {
                        layer.add_attr(Attr::new(&OPTIONS_TS_ATTR, offset..offset + len));
                        layer.add_attr(Attr::new(&OPTIONS_TS_MY_ATTR, offset + 2..offset + 6));
                        layer.add_attr(Attr::new(&OPTIONS_TS_ECHO_ATTR, offset + 6..offset + 10));
                    }
                    _ => {}
                }
                offset += len;
            }

            let payload = layer.data().try_get(data_offset..)?;
            layer.add_payload(Payload::new(payload, "@data:tcp"));
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct TcpDecoder {}

impl Decoder for TcpDecoder {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(TcpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref OFFSET_ATTR_HEADER: Attr = Attr::new(&OFFSET_ATTR, 12..13);
    static ref TCP_CLASS: LayerClass = LayerClass::builder("tcp")
        .header(Attr::new(&SRC_ATTR, 0..2))
        .header(Attr::new(&DST_ATTR, 2..4))
        .header(Attr::new(&SEQ_ATTR, 4..8))
        .header(Attr::new(&ACK_ATTR, 8..12))
        .header(&OFFSET_ATTR_HEADER)
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
    static ref SRC_ATTR: AttrClass = AttrClass::builder("tcp.src")
        .typ("@tcp:port")
        .cast(cast::UInt16BE())
        .build();
    static ref DST_ATTR: AttrClass = AttrClass::builder("tcp.dst")
        .typ("@tcp:port")
        .cast(cast::UInt16BE())
        .build();
    static ref SEQ_ATTR: AttrClass = AttrClass::builder("tcp.seq")
        .cast(cast::UInt32BE())
        .build();
    static ref ACK_ATTR: AttrClass = AttrClass::builder("tcp.ack")
        .cast(cast::UInt32BE())
        .build();
    static ref OFFSET_ATTR: AttrClass = AttrClass::builder("tcp.dataOffset")
        .cast(cast::UInt8().map(|v| v >> 4))
        .build();
    static ref FLAGS_ATTR: AttrClass = AttrClass::builder("tcp.flags")
        .typ("@flags")
        .cast(cast::UInt16BE().map(|v| v & 0xfff))
        .build();
    static ref FLAGS_NS_ATTR: AttrClass = AttrClass::builder("tcp.flags.ns")
        .cast(cast::UInt8().map(|v| (v & 0b0000_0001) != 0))
        .build();
    static ref FLAGS_CWR_ATTR: AttrClass = AttrClass::builder("tcp.flags.cwr")
        .cast(cast::UInt8().map(|v| (v & 0b1000_0000) != 0))
        .build();
    static ref FLAGS_ECE_ATTR: AttrClass = AttrClass::builder("tcp.flags.ece")
        .cast(cast::UInt8().map(|v| (v & 0b0100_0000) != 0))
        .build();
    static ref FLAGS_URG_ATTR: AttrClass = AttrClass::builder("tcp.flags.urg")
        .cast(cast::UInt8().map(|v| (v & 0b0010_0000) != 0))
        .build();
    static ref FLAGS_ACK_ATTR: AttrClass = AttrClass::builder("tcp.flags.ack")
        .cast(cast::UInt8().map(|v| (v & 0b0001_0000) != 0))
        .build();
    static ref FLAGS_PSH_ATTR: AttrClass = AttrClass::builder("tcp.flags.psh")
        .cast(cast::UInt8().map(|v| (v & 0b0000_1000) != 0))
        .build();
    static ref FLAGS_RST_ATTR: AttrClass = AttrClass::builder("tcp.flags.rst")
        .cast(cast::UInt8().map(|v| (v & 0b0000_0100) != 0))
        .build();
    static ref FLAGS_SYN_ATTR: AttrClass = AttrClass::builder("tcp.flags.syn")
        .cast(cast::UInt8().map(|v| (v & 0b0000_0010) != 0))
        .build();
    static ref FLAGS_FIN_ATTR: AttrClass = AttrClass::builder("tcp.flags.fin")
        .cast(cast::UInt8().map(|v| (v & 0b0000_0001) != 0))
        .build();
    static ref WINDOW_ATTR: AttrClass = AttrClass::builder("tcp.window")
        .cast(cast::UInt16BE())
        .build();
    static ref CHECKSUM_ATTR: AttrClass = AttrClass::builder("tcp.checksum")
        .cast(cast::UInt16BE())
        .build();
    static ref URGENT_ATTR: AttrClass = AttrClass::builder("tcp.urgent")
        .cast(cast::UInt16BE())
        .build();
    static ref OPTIONS_ATTR: AttrClass = AttrClass::builder("tcp.options")
        .typ("@nested")
        .cast(cast::Const(true))
        .build();
    static ref OPTIONS_NOP_ATTR: AttrClass = AttrClass::builder("tcp.options.nop")
        .typ("@novalue")
        .cast(cast::Const(true))
        .build();
    static ref OPTIONS_MSS_ATTR: AttrClass = AttrClass::builder("tcp.options.mss")
        .cast(cast::Ranged(cast::UInt16BE(), 2..))
        .build();
    static ref OPTIONS_SCALE_ATTR: AttrClass = AttrClass::builder("tcp.options.scale")
        .cast(cast::Ranged(cast::UInt8(), 2..))
        .build();
    static ref OPTIONS_SACKP_ATTR: AttrClass =
        AttrClass::builder("tcp.options.selectiveAckPermitted")
            .typ("@novalue")
            .cast(cast::Const(true))
            .build();
    static ref OPTIONS_SACK_ATTR: AttrClass = AttrClass::builder("tcp.options.selectiveAck")
        .cast(cast::Ranged(cast::ByteSlice(), 2..))
        .build();
    static ref OPTIONS_TS_ATTR: AttrClass = AttrClass::builder("tcp.options.ts")
        .typ("@nested")
        .cast(cast::Const(true))
        .build();
    static ref OPTIONS_TS_MY_ATTR: AttrClass = AttrClass::builder("tcp.options.ts.my")
        .cast(cast::UInt32BE())
        .build();
    static ref OPTIONS_TS_ECHO_ATTR: AttrClass = AttrClass::builder("tcp.options.ts.echo")
        .cast(cast::UInt32BE())
        .build();
}

genet_decoders!(TcpDecoder {});
