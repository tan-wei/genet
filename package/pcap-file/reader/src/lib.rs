extern crate byteorder;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate serde_derive;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use genet_sdk::{
    io::{Reader, ReaderWorker},
    prelude::*,
};
use std::{
    fs::File,
    io::{self, BufReader, Error, ErrorKind, Read},
};

#[derive(Deserialize)]
struct Arg {
    file: String,
}

#[derive(Clone)]
struct PcapFileReader {}

impl Reader for PcapFileReader {
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<ReaderWorker>> {
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

        let link_class = Fixed::new(layer_class!(format!("[link-{}]", network), 
                header: Attr::with_value(&TYPE_CLASS, 0..0, i64::from(network))
            ));

        Ok(Box::new(PcapFileReaderWorker {
            le,
            nsec,
            reader,
            link_class,
        }))
    }

    fn id(&self) -> &str {
        "pcap-file"
    }
}

struct PcapFileReaderWorker {
    le: bool,
    nsec: bool,
    reader: BufReader<File>,
    link_class: Fixed<LayerClass>,
}

impl PcapFileReaderWorker {
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

        let mut data = Vec::<u8>::with_capacity(inc_len as usize);
        unsafe {
            data.set_len(inc_len as usize);
        }
        self.reader.read_exact(&mut data)?;

        let payload = ByteSlice::from(data);
        let mut layer = Layer::new(self.link_class.clone(), payload);

        layer.add_attr(Attr::with_value(&LENGTH_CLASS, 0..0, u64::from(orig_len)));
        layer.add_attr(Attr::with_value(
            &TS_CLASS,
            0..0,
            f64::from(ts_sec) + f64::from(ts_usec) / 1_000_000f64,
        ));
        layer.add_attr(Attr::with_value(&TS_SEC_CLASS, 0..0, u64::from(ts_sec)));
        layer.add_attr(Attr::with_value(&TS_USEC_CLASS, 0..0, u64::from(ts_usec)));

        Ok(layer)
    }
}

const BLOCK_SIZE: usize = 1024;

impl ReaderWorker for PcapFileReaderWorker {
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

def_attr_class!(TYPE_CLASS, "link.type");
def_attr_class!(LENGTH_CLASS, "link.length");
def_attr_class!(TS_CLASS, "link.timestamp",
    typ: "@datetime:unix"
);
def_attr_class!(TS_SEC_CLASS, "link.timestamp.sec");
def_attr_class!(TS_USEC_CLASS, "link.timestamp.usec");

genet_readers!(PcapFileReader {});
