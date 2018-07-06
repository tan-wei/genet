extern crate byteorder;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate serde_derive;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};

use genet_sdk::{
    attr::{Attr, AttrBuilder, AttrClass},
    context::Context,
    io::{Reader, ReaderWorker},
    layer::{Layer, LayerBuilder, LayerClass},
    ptr::Ptr,
    result::Result,
    slice::Slice,
    token,
    variant::Variant,
};

use std::{
    fs::File,
    io::{self, BufRead, BufReader, Error, ErrorKind, Read},
    mem,
    process::{Child, ChildStdout, Command, Stdio},
    slice,
};

#[derive(Deserialize)]
struct Arg {
    file: String,
}

#[derive(Clone)]
struct PcapFileReader {}

impl Reader for PcapFileReader {
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<ReaderWorker>> {
        let arg: Arg = serde_json::from_str(arg).unwrap();
        let file = File::open(&arg.file)?;
        let mut reader = BufReader::new(file);

        let magic_number = reader.read_u32::<BigEndian>()?;

        let (le, nsec) = match magic_number {
            0xd4c3b2a1 => Ok((true, false)),
            0xa1b2c3d4 => Ok((false, false)),
            0x4d3cb2a1 => Ok((true, true)),
            0xa1b23c4d => Ok((false, true)),
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

        let link_class = LayerBuilder::new(format!("[link-{}]", network))
            .header(Attr::with_value(&TYPE_CLASS, 0..0, network as i64))
            .build();

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
    link_class: Ptr<LayerClass>,
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

        let payload = Slice::from(data);
        let mut layer = Layer::new(&self.link_class, payload);

        layer.add_attr(Attr::with_value(&LENGTH_CLASS, 0..0, orig_len as u64));
        layer.add_attr(Attr::with_value(
            &TS_CLASS,
            0..0,
            ts_sec as f64 + ts_usec as f64 / 1000_000f64,
        ));
        layer.add_attr(Attr::with_value(&TS_SEC_CLASS, 0..0, ts_sec as u64));
        layer.add_attr(Attr::with_value(&TS_USEC_CLASS, 0..0, ts_usec as u64));

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

lazy_static! {
    static ref TYPE_CLASS: Ptr<AttrClass> = AttrBuilder::new("link.type").build();
    static ref LENGTH_CLASS: Ptr<AttrClass> = AttrBuilder::new("link.length").build();
    static ref TS_CLASS: Ptr<AttrClass> = AttrBuilder::new("link.timestamp")
        .typ("@datetime:unix")
        .build();
    static ref TS_SEC_CLASS: Ptr<AttrClass> = AttrBuilder::new("link.timestamp.sec").build();
    static ref TS_USEC_CLASS: Ptr<AttrClass> = AttrBuilder::new("link.timestamp.usec").build();
}

genet_readers!(PcapFileReader {});
