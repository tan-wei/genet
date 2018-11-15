extern crate byteorder;
extern crate genet_sdk;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate serde_derive;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt, WriteBytesExt};
use genet_sdk::{cast, prelude::*, reader::*};
use std::{
    fs::File,
    io::{self, BufReader, Cursor, Error, ErrorKind, Read},
};

#[derive(Deserialize)]
struct Arg {
    file: String,
}

#[derive(Clone)]
struct PcapFileReader {}

impl Reader for PcapFileReader {
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<Worker>> {
        let arg: Arg = serde_json::from_str(arg)?;
        let file = File::open(&arg.file)?;
        let mut reader = BufReader::new(file);

        let magic_number = reader.read_u32::<BigEndian>()?;

        let (le, nsec) = match magic_number {
            0xd4c3_b2a1 => Ok((true, false)),
            0xa1b2_c3d4 => Ok((false, false)),
            0x4d3c_b2a1 => Ok((true, true)),
            0xa1b2_3c4d => Ok((false, true)),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong magic number")),
        }?;

        let (_ver_major, _var_minor, _thiszone, _sigfigs, _snaplen, network) = if le {
            (
                reader.read_u16::<LittleEndian>()?,
                reader.read_u16::<LittleEndian>()?,
                reader.read_i32::<LittleEndian>()?,
                reader.read_u32::<LittleEndian>()?,
                reader.read_u32::<LittleEndian>()?,
                reader.read_u32::<LittleEndian>()?,
            )
        } else {
            (
                reader.read_u16::<BigEndian>()?,
                reader.read_u16::<BigEndian>()?,
                reader.read_i32::<BigEndian>()?,
                reader.read_u32::<BigEndian>()?,
                reader.read_u32::<BigEndian>()?,
                reader.read_u32::<BigEndian>()?,
            )
        };

        let link_class = Fixed::new(layer_class!(
            format!("[link-{}]", network),
            header: attr!(&TYPE_CLASS, range: 0..4),
            header: attr!(&PAYLOAD_LENGTH_CLASS, range: 4..8),
            header: attr!(&ORIG_LENGTH_CLASS, range: 8..12),
            header: attr!(&TS_CLASS, range: 12..20),
            header: attr!(&TS_SEC_CLASS, range: 12..16),
            header: attr!(&TS_USEC_CLASS, range: 16..20)
        ));

        Ok(Box::new(PcapFileWorker {
            le,
            nsec,
            reader,
            link: network,
            link_class,
        }))
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "app.genet.reader.pcap-file".into(),
            filters: vec![FileType::new("Pcap File", &["pcap"])],
            ..Metadata::default()
        }
    }
}

struct PcapFileWorker {
    le: bool,
    nsec: bool,
    reader: BufReader<File>,
    link: u32,
    link_class: Fixed<LayerClass>,
}

impl PcapFileWorker {
    fn read_one(&mut self) -> io::Result<Layer> {
        let (ts_sec, mut ts_usec, inc_len, orig_len) = if self.le {
            (
                self.reader.read_u32::<LittleEndian>()?,
                self.reader.read_u32::<LittleEndian>()?,
                self.reader.read_u32::<LittleEndian>()?,
                self.reader.read_u32::<LittleEndian>()?,
            )
        } else {
            (
                self.reader.read_u32::<BigEndian>()?,
                self.reader.read_u32::<BigEndian>()?,
                self.reader.read_u32::<BigEndian>()?,
                self.reader.read_u32::<BigEndian>()?,
            )
        };

        if !self.nsec {
            ts_usec *= 1000;
        }

        let size = inc_len as usize;
        let metalen = 20;
        let mut data = vec![0u8; size + metalen];
        self.reader.read_exact(&mut data[metalen..])?;

        let mut cur = Cursor::new(data);
        cur.write_u32::<BigEndian>(self.link)?;
        cur.write_u32::<BigEndian>(inc_len)?;
        cur.write_u32::<BigEndian>(orig_len)?;
        cur.write_u32::<BigEndian>(ts_sec)?;
        cur.write_u32::<BigEndian>(ts_usec)?;

        let payload = ByteSlice::from(cur.into_inner());
        let mut layer = Layer::new(self.link_class.clone(), payload);
        layer.add_payload(Payload::new(payload.try_get(metalen..)?, "@data:link"));
        Ok(layer)
    }
}

const BLOCK_SIZE: usize = 65535;

impl Worker for PcapFileWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut layers = Vec::with_capacity(BLOCK_SIZE);
        for _ in 0..BLOCK_SIZE {
            match self.read_one() {
                Ok(layer) => layers.push(layer),
                Err(err) => {
                    if layers.is_empty() {
                        return Err(err.into());
                    }
                }
            }
        }
        Ok(layers)
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

genet_readers!(PcapFileReader {});
