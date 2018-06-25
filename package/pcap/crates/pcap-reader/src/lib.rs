extern crate genet_abi;
extern crate pcap;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate serde_derive;

use genet_sdk::{
    context::Context,
    io::{Reader, ReaderWorker},
    layer::{Layer, LayerClass, LayerClassBuilder},
    ptr::Ptr,
    result::Result,
    token,
};
use pcap::Header;

use std::io::{BufRead, BufReader, Read};
use std::iter;
use std::mem;
use std::process::{Child, ChildStdout, Command, Stdio};
use std::slice;

#[derive(Deserialize)]
struct Arg {
    cmd: String,
    args: Vec<String>,
    link: u32,
}

#[derive(Clone)]
struct PcapReader {}

impl Reader for PcapReader {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Box<ReaderWorker> {
        let arg: Arg = serde_json::from_str(arg).unwrap();
        let mut child = Command::new(&arg.cmd)
            .args(&arg.args)
            .stdout(Stdio::piped())
            .spawn()
            .expect("failed to execute pcap_cli");
        let reader = BufReader::new(child.stdout.take().unwrap());
        let link_class =
            LayerClassBuilder::new(token::get(&format!("[link-{}]", arg.link))).build();
        Box::new(PcapReaderWorker {
            child,
            reader,
            link_class,
        })
    }

    fn id(&self) -> &str {
        "pcap"
    }
}

struct PcapReaderWorker {
    child: Child,
    reader: BufReader<ChildStdout>,
    link_class: Ptr<LayerClass>,
}

impl ReaderWorker for PcapReaderWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut header = String::new();
        let _ = self.reader.read_line(&mut header);
        let header: Header = serde_json::from_str(&header).unwrap();
        let mut data = vec![0u8; header.datalen as usize];
        self.reader.read_exact(&mut data);
        let payload = unsafe { slice::from_raw_parts(data.as_ptr(), data.len()) };
        mem::forget(data);
        Ok(vec![Layer::new(&self.link_class, payload)])
    }
}

impl Drop for PcapReaderWorker {
    fn drop(&mut self) {
        let _ = self.child.kill();
    }
}

genet_readers!(PcapReader {});
