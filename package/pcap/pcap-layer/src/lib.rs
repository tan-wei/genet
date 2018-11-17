extern crate genet_sdk;

use genet_sdk::{decoder::*, prelude::*, cast};

struct PcapLayerWorker {
    class: Option<Fixed<LayerClass>>,
}

impl Worker for PcapLayerWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        _stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        let data;

        if parent.id() == token!("[pcap]") {
            data = parent.data();
        } else {
            return Ok(Status::Skip);
        }

        if self.class.is_none() {
            let link: u32 = TYPE_CLASS_ATTR.try_get(&parent)?.try_into()?;
            let link_class = Fixed::new(layer_class!(
                format!("[link-{}]", link),
                header: attr!(&TYPE_CLASS, range: 0..4),
                header: attr!(&PAYLOAD_LENGTH_CLASS, range: 4..8),
                header: attr!(&ORIG_LENGTH_CLASS, range: 8..12),
                header: attr!(&TS_CLASS, range: 12..20),
                header: attr!(&TS_SEC_CLASS, range: 12..16),
                header: attr!(&TS_USEC_CLASS, range: 16..20)
            ));
            self.class = Some(link_class);
        }

        let mut layer = Layer::new(self.class.as_ref().unwrap().clone(), data);
        layer.add_payload(Payload::new(data.try_get(20..)?, "@data:link"));
        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct PcapLayerDecoder {}

impl Decoder for PcapLayerDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(PcapLayerWorker { class: None })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_attr_class!(TYPE_CLASS, "link.type", cast: cast::UInt32BE());
def_attr_class!(
    PAYLOAD_LENGTH_CLASS,
    "link.payloadLength",
    cast: cast::UInt32BE()
);
def_attr_class!(
    ORIG_LENGTH_CLASS,
    "link.originalLength",
    cast: cast::UInt32BE()
);
def_attr_class!(TS_CLASS, "link.timestamp",
    typ: "@datetime:unix", 
    cast: cast::UInt64BE().map(|v| (v >> 32) as f64 + (v & 0xffff_ffff) as f64 / 1_000_000f64)
);
def_attr_class!(TS_SEC_CLASS, "link.timestamp.sec", cast: cast::UInt32BE());
def_attr_class!(TS_USEC_CLASS, "link.timestamp.usec", cast: cast::UInt32BE());

def_attr!(TYPE_CLASS_ATTR,  &TYPE_CLASS, range: 0..4);

genet_decoders!(PcapLayerDecoder {});
