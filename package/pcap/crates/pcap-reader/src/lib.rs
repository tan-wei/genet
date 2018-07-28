extern crate pcap;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate serde_derive;

use genet_sdk::{
    io::{Reader, ReaderWorker},
    prelude::*,
};
use pcap::Header;

use std::{
    io::{BufRead, BufReader, Error, ErrorKind, Read},
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
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<ReaderWorker>> {
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
        let link_class = Fixed::new(
            LayerBuilder::new(format!("[link-{}]", arg.link))
                .header(Attr::with_value(&TYPE_CLASS, 0..0, u64::from(arg.link)))
                .build(),
        );
        Ok(Box::new(PcapReaderWorker {
            child,
            reader,
            link_class,
        }))
    }

    fn id(&self) -> &str {
        "pcap"
    }
}

struct PcapReaderWorker {
    child: Child,
    reader: BufReader<ChildStdout>,
    link_class: Fixed<LayerClass>,
}

impl ReaderWorker for PcapReaderWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut header = String::new();
        self.reader.read_line(&mut header)?;
        let header = header.trim();
        if header.is_empty() {
            return Ok(vec![]);
        }
        let header: Header = serde_json::from_str(header)?;
        let mut data = vec![0u8; header.datalen as usize];
        self.reader.read_exact(&mut data)?;
        let payload = ByteSlice::from(data);
        let mut layer = Layer::new(self.link_class.clone(), payload);
        layer.add_attr(Attr::with_value(
            &LENGTH_CLASS,
            0..0,
            u64::from(header.actlen),
        ));
        layer.add_attr(Attr::with_value(
            &TS_CLASS,
            0..0,
            f64::from(header.ts_sec) + f64::from(header.ts_usec) / 1_000_000f64,
        ));
        layer.add_attr(Attr::with_value(
            &TS_SEC_CLASS,
            0..0,
            u64::from(header.ts_sec),
        ));
        layer.add_attr(Attr::with_value(
            &TS_USEC_CLASS,
            0..0,
            u64::from(header.ts_usec),
        ));
        Ok(vec![layer])
    }
}

impl Drop for PcapReaderWorker {
    fn drop(&mut self) {
        let _ = self.child.kill();
    }
}

lazy_static! {
    static ref TYPE_CLASS: AttrClass = AttrBuilder::new("link.type").build();
    static ref LENGTH_CLASS: AttrClass = AttrBuilder::new("link.length").build();
    static ref TS_CLASS: AttrClass = AttrBuilder::new("link.timestamp")
        .typ("@datetime:unix")
        .build();
    static ref TS_SEC_CLASS: AttrClass = AttrBuilder::new("link.timestamp.sec").build();
    static ref TS_USEC_CLASS: AttrClass = AttrBuilder::new("link.timestamp.usec").build();
}

genet_readers!(PcapReader {});
