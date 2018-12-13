use genet_sdk::{cast, decoder::*, prelude::*};

struct TcpWorker {}

impl Worker for TcpWorker {
    fn decode(&mut self, parent: &mut Parent) -> Result<Status> {
        let data;

        if let Some(payload) = parent.payloads().find(|p| p.id() == token!("@data:tcp")) {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&TCP_CLASS, data);

        let data_offset: usize = OFFSET_ATTR.try_get(&layer)?.try_into()?;
        let data_offset = data_offset * 4;
        let mut offset = 20;

        while offset < data_offset {
            let typ = layer.data().try_get(offset)?;
            if typ <= 1 {
                if typ == 1 {
                    layer.add_attr(&OPTIONS_NOP_ATTR, offset..offset + 1);
                }
                offset += 1;
                continue;
            }
            let len = layer.data().try_get(offset + 1)? as usize;
            match typ {
                2 => {
                    layer.add_attr(&OPTIONS_MSS_ATTR, offset + 2..offset + len);
                }
                3 => {
                    layer.add_attr(&OPTIONS_SCALE_ATTR, offset + 2..offset + len);
                }
                4 => {
                    layer.add_attr(&OPTIONS_SACKP_ATTR, offset..offset + len);
                }
                5 => {
                    layer.add_attr(&OPTIONS_SACK_ATTR, offset + 2..offset + len);
                }
                8 => {
                    layer.add_attr(&OPTIONS_TS_ATTR, offset..offset + len);
                    layer.add_attr(&OPTIONS_TS_MY_ATTR, offset + 2..offset + 6);
                    layer.add_attr(&OPTIONS_TS_ECHO_ATTR, offset + 6..offset + 10);
                }
                _ => {}
            }
            offset += len;
        }
        layer.add_attr(&OPTIONS_ATTR, 20..offset);

        let payload = layer.data().try_get(data_offset..)?;
        layer.add_payload(Payload::new(payload, "@data:tcp"));

        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct TcpDecoder {}

impl Decoder for TcpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(TcpWorker {})
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "tcp".into(),
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(TCP_CLASS, &TCP_ATTR);

def_attr_class!(
    TCP_ATTR,
    "tcp",
    child: &SRC_ATTR,
    child: &DST_ATTR,
    child: &SEQ_ATTR,
    child: &ACK_ATTR,
    child: &OFFSET_ATTR,
    child: &FLAGS_ATTR,
    child: &FLAGS_NS_ATTR,
    child: &FLAGS_CWR_ATTR,
    child: &FLAGS_ECE_ATTR,
    child: &FLAGS_URG_ATTR,
    child: &FLAGS_ACK_ATTR,
    child: &FLAGS_PSH_ATTR,
    child: &FLAGS_RST_ATTR,
    child: &FLAGS_SYN_ATTR,
    child: &FLAGS_FIN_ATTR,
    child: &WINDOW_ATTR,
    child: &CHECKSUM_ATTR,
    child: &URGENT_ATTR
);

def_attr_class!(SRC_ATTR, "tcp.src",
    typ: "@tcp:port",
    cast: &cast::UInt16BE(),
    range: 0..2
);

def_attr_class!(DST_ATTR, "tcp.dst",
    typ: "@tcp:port",
    cast: &cast::UInt16BE(),
    range: 2..4
);

def_attr_class!(SEQ_ATTR, "tcp.seq", 
    cast: &cast::UInt32BE(),
    range: 4..8
);

def_attr_class!(ACK_ATTR, "tcp.ack",
    cast: &cast::UInt32BE(),
    range: 8..12
);

def_attr_class!(OFFSET_ATTR, "tcp.dataOffset",
    cast: &cast::UInt8().map(|v| v >> 4),
    range: 12..13
);

def_attr_class!(FLAGS_ATTR, "tcp.flags",
    typ: "@flags",
    cast: &cast::UInt16BE().map(|v| v & 0xfff),
    bit_range: 12 7..16
);

def_attr_class!(FLAGS_NS_ATTR, "tcp.flags.ns",
    cast: &cast::UInt8().map(|v| (v & 0b0000_0001) != 0),
    bit_range: 12 7..8
);
def_attr_class!(FLAGS_CWR_ATTR, "tcp.flags.cwr",
    cast: &cast::UInt8().map(|v| (v & 0b1000_0000) != 0),
    bit_range: 13 0..1
);

def_attr_class!(FLAGS_ECE_ATTR, "tcp.flags.ece",
    cast: &cast::UInt8().map(|v| (v & 0b0100_0000) != 0),
    bit_range: 13 1..2
);

def_attr_class!(FLAGS_URG_ATTR, "tcp.flags.urg",
    cast: &cast::UInt8().map(|v| (v & 0b0010_0000) != 0),
    bit_range: 13 2..3
);

def_attr_class!(FLAGS_ACK_ATTR, "tcp.flags.ack",
    cast: &cast::UInt8().map(|v| (v & 0b0001_0000) != 0),
    bit_range: 13 3..4
);

def_attr_class!(FLAGS_PSH_ATTR, "tcp.flags.psh",
    cast: &cast::UInt8().map(|v| (v & 0b0000_1000) != 0),
    bit_range: 13 4..5
);

def_attr_class!(FLAGS_RST_ATTR, "tcp.flags.rst",
    cast: &cast::UInt8().map(|v| (v & 0b0000_0100) != 0),
    bit_range: 13 5..6
);

def_attr_class!(FLAGS_SYN_ATTR, "tcp.flags.syn",
    cast: &cast::UInt8().map(|v| (v & 0b0000_0010) != 0),
    bit_range: 13 6..7
);

def_attr_class!(FLAGS_FIN_ATTR, "tcp.flags.fin",
    cast: &cast::UInt8().map(|v| (v & 0b0000_0001) != 0),
    bit_range: 13 7..8
);

def_attr_class!(WINDOW_ATTR, "tcp.window", 
    cast: &cast::UInt16BE(),
    range: 14..16
);

def_attr_class!(CHECKSUM_ATTR, "tcp.checksum",
    cast: &cast::UInt16BE(),
    range: 16..18
);

def_attr_class!(URGENT_ATTR, "tcp.urgent",
    cast: &cast::UInt16BE(),
    range: 18..20
);

def_attr_class!(OPTIONS_ATTR, "tcp.options",
    typ: "@nested",
    value: true
);

def_attr_class!(OPTIONS_NOP_ATTR, "tcp.options.nop",
    typ: "@novalue",
    value: true
);

def_attr_class!(OPTIONS_MSS_ATTR, "tcp.options.mss", cast: &cast::UInt16BE());

def_attr_class!(
    OPTIONS_SCALE_ATTR,
    "tcp.options.scale",
    cast: &cast::UInt8()
);

def_attr_class!(OPTIONS_SACKP_ATTR, "tcp.options.selectiveAckPermitted",
    typ: "@novalue",
    value: true
);

def_attr_class!(
    OPTIONS_SACK_ATTR,
    "tcp.options.selectiveAck",
    cast: &cast::ByteSlice()
);

def_attr_class!(OPTIONS_TS_ATTR, "tcp.options.ts",
    typ: "@nested",
    value: true
);

def_attr_class!(
    OPTIONS_TS_MY_ATTR,
    "tcp.options.ts.my",
    cast: &cast::UInt32BE()
);

def_attr_class!(
    OPTIONS_TS_ECHO_ATTR,
    "tcp.options.ts.echo",
    cast: &cast::UInt32BE()
);

genet_decoders!(TcpDecoder {});
