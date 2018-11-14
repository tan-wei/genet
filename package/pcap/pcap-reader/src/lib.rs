extern crate byteorder;
extern crate genet_sdk;
extern crate pcap;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate serde_derive;

use byteorder::{BigEndian, WriteBytesExt};
use genet_sdk::{cast, prelude::*, reader::*};
use pcap::Header;

use std::{
    io::{BufRead, BufReader, Cursor, Error, ErrorKind, Read},
    process::{Child, ChildStdout, Command, Stdio},
};

#[derive(Deserialize)]
struct Arg {
    cmd: String,
    args: Vec<String>,
    link: u32,
}

#[derive(Clone)]
struct PcapReader {}

impl Reader for PcapReader {
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<Worker>> {
        let arg: Arg = serde_json::from_str(arg)?;
        let mut child = Command::new(&arg.cmd)
            .args(&arg.args)
            .stdout(Stdio::piped())
            .spawn()?;
        let reader = BufReader::new(
            child
                .stdout
                .take()
                .ok_or_else(|| Error::new(ErrorKind::Other, "no stdout"))?,
        );
        let link_class = Fixed::new(layer_class!(
            format!("[link-{}]", arg.link),
            header: attr!(&TYPE_CLASS, range: 0..4),
            header: attr!(&PAYLOAD_LENGTH_CLASS, range: 4..8),
            header: attr!(&ORIG_LENGTH_CLASS, range: 8..12),
            header: attr!(&TS_CLASS, range: 12..20),
            header: attr!(&TS_SEC_CLASS, range: 12..16),
            header: attr!(&TS_USEC_CLASS, range: 16..20)
        ));
        Ok(Box::new(PcapWorker {
            child,
            reader,
            link: arg.link,
            link_class,
        }))
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "app.genet.reader.pcap".into(),
            ..Metadata::default()
        }
    }
}

struct PcapWorker {
    child: Child,
    reader: BufReader<ChildStdout>,
    link: u32,
    link_class: Fixed<LayerClass>,
}

impl Worker for PcapWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut header = String::new();
        self.reader.read_line(&mut header)?;
        let header = header.trim();
        if header.is_empty() {
            return Ok(vec![]);
        }
        let header: Header = serde_json::from_str(header)?;
        let size = header.datalen as usize;

        let metalen = 20;
        let mut data = vec![0u8; size + metalen];
        self.reader.read_exact(&mut data[metalen..])?;

        let mut cur = Cursor::new(data);
        cur.write_u32::<BigEndian>(self.link)?;
        cur.write_u32::<BigEndian>(header.datalen)?;
        cur.write_u32::<BigEndian>(header.actlen)?;
        cur.write_u32::<BigEndian>(header.ts_sec)?;
        cur.write_u32::<BigEndian>(header.ts_usec)?;

        let payload = ByteSlice::from(cur.into_inner());
        let mut layer = Layer::new(self.link_class.clone(), payload);
        layer.add_payload(Payload::new(payload.try_get(metalen..)?, "@data:link"));
        Ok(vec![layer])
    }
}

impl Drop for PcapWorker {
    fn drop(&mut self) {
        let _ = self.child.kill();
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

genet_readers!(PcapReader {});
