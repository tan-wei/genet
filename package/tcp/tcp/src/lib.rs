use genet_derive::Attr;
use genet_sdk::{cast, decoder::*, prelude::*};

struct TcpWorker {
    layer: LayerType<Tcp>,
}

impl Worker for TcpWorker {
    fn decode(&mut self, stack: &mut LayerStack) -> Result<Status> {
        let data = stack.top().unwrap().payload();
        let mut layer = Layer::new(&self.layer, &data);

        let data_offset: usize = self.layer.data_offset.try_get(&layer)?.try_into()?;
        let data_offset = data_offset * 4;

        let opt_offset = self.layer.byte_size();
        let mut offset = opt_offset;
        while offset < data_offset {
            let typ = layer.data().try_get(offset)?;
            if typ <= 1 {
                if typ == 1 {
                    layer.add_attr(self.layer.options.nop.class(), offset..offset + 1);
                }
                offset += 1;
                continue;
            }
            let len = layer.data().try_get(offset + 1)? as usize;
            match typ {
                2 => {
                    layer.add_attr(self.layer.options.mss.class(), offset + 2..offset + len);
                }
                3 => {
                    layer.add_attr(self.layer.options.scale.class(), offset + 2..offset + len);
                }
                4 => {
                    layer.add_attr(
                        self.layer.options.selective_ack_permitted.class(),
                        offset..offset + len,
                    );
                }
                5 => {
                    layer.add_attr(
                        self.layer.options.selective_ack.class(),
                        offset + 2..offset + len,
                    );
                }
                8 => {
                    layer.add_attr(self.layer.options.ts.class(), offset + 2..offset + len);
                }
                _ => {}
            }
            offset += len;
        }
        layer.add_attr(self.layer.options.class(), opt_offset..offset);

        stack.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct TcpDecoder {}

impl Decoder for TcpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(TcpWorker {
            layer: LayerType::new("tcp", Tcp::default()),
        })
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "tcp".into(),
            ..Metadata::default()
        }
    }
}

#[derive(Attr, Default)]
struct Tcp {
    #[genet(alias = "_.src", typ = "@tcp:port")]
    src: cast::UInt16BE,

    #[genet(alias = "_.dst", typ = "@tcp:port")]
    dst: cast::UInt16BE,

    seq: cast::UInt32BE,

    ask: cast::UInt32BE,
    #[genet(bit_size = 4, map = "x >> 4")]
    data_offset: Node<cast::UInt8>,

    #[genet(bit_size = 12, typ = "@flags", map = "x & 0xfff")]
    flags: Node<cast::UInt16BE, Flags>,

    window: cast::UInt16BE,

    checksum: cast::UInt16BE,

    urgent: cast::UInt16BE,

    #[genet(detach, byte_size = 1)]
    options: Node<cast::ByteSlice, Options>,
}

#[derive(Attr, Default)]
struct Flags {
    #[genet(bit_size = 3)]
    reserved: cast::Nil,

    ns: cast::BitFlag,

    cwr: cast::BitFlag,

    ece: cast::BitFlag,

    urg: cast::BitFlag,

    ack: cast::BitFlag,

    psh: cast::BitFlag,

    rst: cast::BitFlag,

    syn: cast::BitFlag,

    fin: cast::BitFlag,
}

#[derive(Attr, Default)]
struct Options {
    #[genet(detach)]
    nop: Node<cast::UInt8>,

    #[genet(detach)]
    mss: Node<cast::UInt16BE>,

    #[genet(detach)]
    scale: Node<cast::UInt8>,

    #[genet(detach)]
    selective_ack_permitted: Node<cast::UInt8>,

    #[genet(detach, byte_size = 1)]
    selective_ack: Node<cast::ByteSlice>,

    #[genet(detach, typ = "@nested")]
    ts: Node<Timestamp>,
}

#[derive(Attr, Default)]
struct Timestamp {
    my: cast::UInt32BE,
    echo: cast::UInt32BE,
}

genet_decoders!(TcpDecoder {});
