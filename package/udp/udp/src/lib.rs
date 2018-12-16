use genet_derive::Attr;
use genet_sdk::{cast, decoder::*, prelude::*};

struct UdpWorker {
    layer: LayerType<Udp>,
}

impl Worker for UdpWorker {
    fn decode(&mut self, stack: &mut LayerStack, data: &ByteSlice) -> Result<Status> {
        let layer = Layer::new(self.layer.as_ref().clone(), data);
        stack.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct UdpDecoder {}

impl Decoder for UdpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(UdpWorker {
            layer: LayerType::new("udp", Udp::default()),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "udp".into(),
            ..Metadata::default()
        }
    }
}

#[derive(Attr, Default)]
struct Udp {
    #[genet(alias = "_.src", typ = "@udp:port")]
    src: cast::UInt16BE,

    #[genet(alias = "_.dst", typ = "@udp:port")]
    dst: cast::UInt16BE,

    length: cast::UInt16BE,

    checksum: cast::UInt16BE,
}

genet_decoders!(UdpDecoder {});
