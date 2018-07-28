#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

use genet_sdk::prelude::*;

struct UdpWorker {}

impl Worker for UdpWorker {
    fn analyze(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if let Some(payload) = parent
            .payloads()
            .iter()
            .find(|p| p.id() == token!("@data:udp"))
        {
            let mut layer = Layer::new(&UDP_CLASS, payload.data());
            let payload = payload.data().try_get(8..)?;
            layer.add_payload(payload, "", "");
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct UdpDissector {}

impl Dissector for UdpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(UdpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref UDP_CLASS: LayerClass = LayerClass::builder("udp")
        .alias("_.src", "udp.src")
        .alias("_.dst", "udp.dst")
        .header(Attr::new(&SRC_ATTR, 0..2))
        .header(Attr::new(&DST_ATTR, 2..4))
        .header(Attr::new(&LEN_ATTR, 4..6))
        .header(Attr::new(&CHECKSUM_ATTR, 6..8))
        .build();
    static ref SRC_ATTR: AttrClass = AttrClass::builder("udp.src")
        .typ("@udp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref DST_ATTR: AttrClass = AttrClass::builder("udp.dst")
        .typ("@udp:port")
        .decoder(decoder::UInt16BE())
        .build();
    static ref LEN_ATTR: AttrClass = AttrClass::builder("udp.length")
        .decoder(decoder::UInt16BE())
        .build();
    static ref CHECKSUM_ATTR: AttrClass = AttrClass::builder("udp.checksum")
        .decoder(decoder::UInt16BE())
        .build();
}

genet_dissectors!(UdpDissector {});
