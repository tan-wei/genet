use genet_derive::Attr;
use genet_sdk::{decoder::*, prelude::*};

/// Ethernet
#[derive(Attr)]
struct Eth {
    /// Source Hardware Address
    #[genet(alias = "_.src", typ = "@eth:mac", byte_size = 6)]
    src: ByteSlice,

    /// Destination Hardware Address
    #[genet(alias = "_.dst", typ = "@eth:mac", byte_size = 6)]
    dst: ByteSlice,

    #[genet(cond = "x <= 1500")]
    len: u16,

    #[genet(cond = "x > 1500", align_before, map = "x as u16")]
    r#type: Enum<u16, EthTypeEnum>,
}

#[derive(Attr)]
enum EthTypeEnum {
    IPv4,
    ARP,
    WOL,
    IPv6,
    EAP,
    Unknown,
}

impl Default for EthTypeEnum {
    fn default() -> Self {
        EthTypeEnum::Unknown
    }
}

impl From<u16> for EthTypeEnum {
    fn from(data: u16) -> EthTypeEnum {
        match data {
            0x0800 => EthTypeEnum::IPv4,
            0x0806 => EthTypeEnum::ARP,
            0x0842 => EthTypeEnum::WOL,
            0x86DD => EthTypeEnum::IPv6,
            0x888E => EthTypeEnum::EAP,
            _ => EthTypeEnum::Unknown,
        }
    }
}

struct EthWorker {
    layer: LayerType<Eth>,
}

impl Worker for EthWorker {
    fn decode(&mut self, stack: &mut LayerStack) -> Result<Status> {
        if stack.id() == token!("[link-1]") {
            let data = stack.top().unwrap().payload();
            let layer = Layer::new(&self.layer, &data);
            stack.add_child(layer);
            Ok(Status::Done)
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct EthDecoder {}

impl Decoder for EthDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(EthWorker {
            layer: LayerType::new("eth"),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "eth".into(),
            ..Metadata::default()
        }
    }
}

genet_decoders!(EthDecoder {});

#[test]
fn session() {
    let leyer: LayerType<Tcp> = LayerType::new("tcp");
    println!("{:#?}", leyer.as_ref());
}

struct TcpWorker {
    layer: LayerType<Tcp>,
}

impl Worker for TcpWorker {
    fn decode(&mut self, stack: &mut LayerStack) -> Result<Status> {
        let data = stack.top().unwrap().payload();
        let mut layer = Layer::new(&self.layer, &data);

        let data_offset = self.layer.data_offset.try_get(&layer)? as usize;
        let data_offset = data_offset * 4;

        let opt_offset = self.layer.byte_size();
        let mut offset = opt_offset;
        while offset < data_offset {
            let typ = layer.data().try_get(offset)?;
            if typ <= 1 {
                if typ == 1 {
                    layer.add_attr(&self.layer.options.nop, offset..offset + 1);
                }
                offset += 1;
                continue;
            }
            let len = layer.data().try_get(offset + 1)? as usize;
            match typ {
                2 => {
                    layer.add_attr(&self.layer.options.mss, offset + 2..offset + len);
                }
                3 => {
                    layer.add_attr(&self.layer.options.scale, offset + 2..offset + len);
                }
                4 => {
                    layer.add_attr(
                        &self.layer.options.selective_ack_permitted,
                        offset..offset + len,
                    );
                }
                5 => {
                    layer.add_attr(&self.layer.options.selective_ack, offset + 2..offset + len);
                }
                8 => {
                    layer.add_attr(&self.layer.options.ts, offset + 2..offset + len);
                }
                _ => {}
            }
            offset += len;
        }
        layer.add_attr(&self.layer.options, opt_offset..offset);

        stack.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct TcpDecoder {}

impl Decoder for TcpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(TcpWorker {
            layer: LayerType::new("tcp"),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "tcp".into(),
            ..Metadata::default()
        }
    }
}

#[derive(Attr)]
struct Tcp {
    #[genet(alias = "_.src", typ = "@tcp:port")]
    src: u16,

    #[genet(alias = "_.dst", typ = "@tcp:port")]
    dst: u16,

    seq: u32,

    ack: u32,

    #[genet(bit_size = 4, map = "x >> 4")]
    data_offset: Node<u8>,

    #[genet(bit_size = 12, typ = "@flags", map = "x & 0xfff")]
    flags: Node<u16, Flags>,

    window: u16,

    checksum: u16,

    urgent: u16,

    #[genet(byte_size = 1)]
    options: Node<ByteSlice, Options>,
}

#[derive(Attr)]
struct Flags {
    #[genet(bit_size = 3, skip)]
    reserved: u8,

    ns: BitFlag,

    cwr: BitFlag,

    ece: BitFlag,

    urg: BitFlag,

    ack: BitFlag,

    psh: BitFlag,

    rst: BitFlag,

    syn: BitFlag,

    fin: BitFlag,
}

#[derive(Attr)]
struct Options {
    nop: Node<u8>,

    #[genet(detach)]
    mss: Node<u16>,

    #[genet(detach)]
    scale: Node<u8>,

    #[genet(detach)]
    selective_ack_permitted: Node<u8>,

    #[genet(detach, byte_size = 1)]
    selective_ack: Node<ByteSlice>,

    #[genet(typ = "@nested")]
    ts: Node<Timestamp>,
}

#[derive(Attr)]
struct Timestamp {
    my: u32,
    echo: u32,
}
