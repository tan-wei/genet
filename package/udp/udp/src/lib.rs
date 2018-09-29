#[macro_use]
extern crate genet_sdk;

use genet_sdk::prelude::*;

struct UdpWorker {}

impl Worker for UdpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut MutLayer,
    ) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:udp"))
        {
            let mut layer = Layer::new(&UDP_CLASS, payload.data());
            let payload = payload.data().try_get(8..)?;
            layer.add_payload(Payload::new(payload, ""));
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct UdpDecoder {}

impl Decoder for UdpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(UdpWorker {})
    }

    fn execution_type(&self) -> ExecType {
        ExecType::ParallelSync
    }
}

def_layer_class!(UDP_CLASS, "udp",
    alias: "_.src" "udp.src",
    alias: "_.dst" "udp.dst",
    header: attr!(&SRC_ATTR, range: 0..2),
    header: attr!(&DST_ATTR, range: 2..4),
    header: attr!(&LEN_ATTR, range: 4..6),
    header: attr!(&CHECKSUM_ATTR, range: 6..8)
);

def_attr_class!(SRC_ATTR, "udp.src",
    typ: "@udp:port",
    cast: cast::UInt16BE()
);

def_attr_class!(DST_ATTR, "udp.dst",
    typ: "@udp:port",
    cast: cast::UInt16BE()
);

def_attr_class!(LEN_ATTR, "udp.length", cast: cast::UInt16BE());

def_attr_class!(CHECKSUM_ATTR, "udp.checksum", cast: cast::UInt16BE());

genet_decoders!(UdpDecoder {});
