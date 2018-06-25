#[macro_use]
extern crate genet_sdk;
extern crate genet_abi;
extern crate pcap;

#[macro_use]
extern crate lazy_static;

extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate serde_derive;

use genet_sdk::{
    context::Context,
    io::{Reader, ReaderWorker},
    layer::{Layer, LayerClass, LayerClassBuilder},
    ptr::Ptr,
    result::Result,
};
use pcap::Header;

use std::iter;
use std::io::{Read, BufRead, BufReader};
use std::process::{Command, Child, Stdio, ChildStdout};

#[derive(Deserialize)]
struct Arg {
    cmd: String,
    args: Vec<String>,
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
                           .expect("failed to execute child");
        let reader = BufReader::new(child.stdout.take().unwrap());
        Box::new(PcapReaderWorker { child, reader })
    }

    fn id(&self) -> &str {
        "pcap"
    }
}

struct PcapReaderWorker {
	child: Child,
    reader: BufReader<ChildStdout>
}

impl ReaderWorker for PcapReaderWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut header = String::new();
        let _ = self.reader.read_line(&mut header);
        let header: Header = serde_json::from_str(&header).unwrap();
        let mut payload = vec![0u8; header.datalen as usize];
        self.reader.read_exact(&mut payload);
        /*
    	self.child.stdout.as_mut().unwrap();
        let layers = iter::repeat(())
            .take(100)
            .map(|_| Layer::new(&ETH_CLASS, tcp_ipv4_pcap()))
            .collect();
        */
        Ok(vec![])
    }
}

impl Drop for PcapReaderWorker {
	fn drop(&mut self) {
		let _ = self.child.kill();
	}
}

lazy_static! {
    static ref ETH_CLASS: Ptr<LayerClass> = LayerClassBuilder::new(token!("[link-1]")).build();
}

genet_readers!(PcapReader {});