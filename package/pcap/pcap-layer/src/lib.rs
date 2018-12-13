use genet_sdk::{cast, decoder::*, prelude::*};

struct PcapLayerWorker {
    class: Option<Fixed<LayerClass>>,
}

impl Worker for PcapLayerWorker {
    fn decode(&mut self, parent: &mut Parent) -> Result<Status> {
        let data;

        if parent.id() == token!("[pcap]") {
            data = parent.data();
        } else {
            return Ok(Status::Skip);
        }

        if self.class.is_none() {
            let link: u32 = TYPE_CLASS.try_get(&parent)?.try_into()?;

            let id = format!("[link-{}]", link);
            let attr = Fixed::new(attr_class!(
                id,
                child: &TYPE_CLASS,
                child: &PAYLOAD_LENGTH_CLASS,
                child: &ORIG_LENGTH_CLASS,
                child: &TS_CLASS,
                child: &TS_SEC_CLASS,
                child: &TS_USEC_CLASS
            ));

            let link_class = Fixed::new(layer_class!(attr));
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
            id: "pcap_layer".into(),
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_attr_class!(
    LINK_ATTR,
    "link",
    child: &TYPE_CLASS,
    child: &PAYLOAD_LENGTH_CLASS,
    child: &ORIG_LENGTH_CLASS,
    child: &TS_CLASS,
    child: &TS_SEC_CLASS,
    child: &TS_USEC_CLASS
);

def_attr_class!(TYPE_CLASS, "link.type",
    cast: &cast::UInt32BE(),
    range: 0..4
);

def_attr_class!(
    PAYLOAD_LENGTH_CLASS,
    "link.payloadLength",
    cast: &cast::UInt32BE(),
    range: 4..8
);

def_attr_class!(
    ORIG_LENGTH_CLASS,
    "link.originalLength",
    cast: &cast::UInt32BE(),
    range: 8..12
);

def_attr_class!(TS_CLASS, "link.timestamp",
    typ: "@datetime:unix", 
    cast: &cast::UInt64BE().map(|v| (v >> 32) as f64 + (v & 0xffff_ffff) as f64 / 1_000_000f64),
    range: 12..20
);

def_attr_class!(TS_SEC_CLASS, "link.timestamp.sec",
    cast: &cast::UInt32BE(),
    range: 12..16
);

def_attr_class!(TS_USEC_CLASS, "link.timestamp.usec",
    cast: &cast::UInt32BE(),
    range: 16..20
);

genet_decoders!(PcapLayerDecoder {});
