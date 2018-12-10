use genet_sdk::{cast, decoder::*, prelude::*};

struct UdpWorker {}

impl Worker for UdpWorker {
    fn decode(&mut self, _stack: &LayerStack, parent: &mut Parent) -> Result<Status> {
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
            id: "udp".into(),
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(UDP_CLASS, &UDP_ATTR);

def_attr_class!(
    UDP_ATTR,
    "udp",
    child: &SRC_ATTR,
    child: &DST_ATTR,
    child: &LEN_ATTR,
    child: &CHECKSUM_ATTR
);

def_attr_class!(SRC_ATTR, "udp.src",
    typ: "@udp:port",
    alias: "_.src",
    cast: &cast::UInt16BE(),
    range: 0..2
);

def_attr_class!(DST_ATTR, "udp.dst",
    typ: "@udp:port",
    alias: "_.dst",
    cast: &cast::UInt16BE(),
    range: 2..4
);

def_attr_class!(LEN_ATTR, "udp.length", 
    cast: &cast::UInt16BE(),
    range: 4..6
);

def_attr_class!(CHECKSUM_ATTR, "udp.checksum",
    cast: &cast::UInt16BE(),
    range: 6..8
);

genet_decoders!(UdpDecoder {});
