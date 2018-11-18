extern crate genet_sdk;

use genet_sdk::{cast, decoder::*, prelude::*};

struct UdpWorker {}

impl Worker for UdpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        let data;

        if let Some(payload) = parent.payloads().find(|p| p.id() == token!("@data:udp")) {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

        let mut layer = Layer::new(&UDP_CLASS, data);
        let payload = data.try_get(8..)?;
        layer.add_payload(Payload::new(payload, ""));

        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct UdpDecoder {}

impl Decoder for UdpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(UdpWorker {})
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(UDP_CLASS, "udp",
    alias: "_.src" "udp.src",
    alias: "_.dst" "udp.dst",
    header2: &SRC_ATTR,
    header2: &DST_ATTR,
    header2: &LEN_ATTR,
    header2: &CHECKSUM_ATTR
);

def_attr_class!(SRC_ATTR, "udp.src",
    typ: "@udp:port",
    cast: cast::UInt16BE(),
    range: 0..2
);

def_attr_class!(DST_ATTR, "udp.dst",
    typ: "@udp:port",
    cast: cast::UInt16BE(),
    range: 2..4
);

def_attr_class!(LEN_ATTR, "udp.length", 
    cast: cast::UInt16BE(),
    range: 4..6
);

def_attr_class!(CHECKSUM_ATTR, "udp.checksum",
    cast: cast::UInt16BE(),
    range: 6..8
);

genet_decoders!(UdpDecoder {});
