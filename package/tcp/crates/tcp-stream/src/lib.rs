#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::{BTreeMap, HashMap};

#[derive(Debug)]
struct Stream {
    pub id: u64,
    pub seq: i64,
    pub len: usize,
    offset: usize,
    slices: BTreeMap<usize, &'static [u8]>,
}

impl Stream {
    fn new(id: u64) -> Stream {
        return Stream {
            id: id,
            seq: -1,
            len: 0,
            offset: 0,
            slices: BTreeMap::new(),
        };
    }

    fn put(&mut self, start: usize, data: &'static [u8]) {
        if data.len() > 0 {
            self.slices.insert(start, data);
            let mut end = self.offset;
            for (start, slice) in self.slices.iter_mut() {
                if *start < end {}
                end = *start + slice.len();
            }
        }
    }

    fn fetch(&mut self) -> Box<[&'static [u8]]> {
        let mut slices = Vec::<&'static [u8]>::new();
        loop {
            let pos;
            if let Some((key, value)) = self.slices.iter().next() {
                pos = *key;
                if *key == self.offset {
                    self.offset += value.len();
                    slices.push(value);
                } else {
                    break;
                }
            } else {
                break;
            }
            self.slices.remove(&pos);
        }
        slices.into_boxed_slice()
    }
}

struct TcpStreamWorker {
    map: HashMap<(&'static [u8], &'static [u8], u32, u32), Stream>,
}

impl TcpStreamWorker {
    fn new() -> TcpStreamWorker {
        TcpStreamWorker {
            map: HashMap::new(),
        }
    }
}

impl Worker for TcpStreamWorker {
    fn analyze(&mut self, _ctx: &mut Context, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("tcp") {
            let parent_src: Slice = parent
                .attr(token!("_.src"))
                .unwrap()
                .get(parent)?
                .try_into()?;

            parent.add_attr(Attr::new(&STREAM_ATTR, 0..0));
            Ok(Status::Done(vec![]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct TcpStreamDissector {}

impl Dissector for TcpStreamDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "serial" {
            Some(Box::new(TcpStreamWorker::new()))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref STREAM_ATTR: Ptr<AttrClass> = AttrBuilder::new("tcp.stream")
        .typ("@novalue")
        .decoder(decoder::Ranged(decoder::UInt8(), 0..0).map(|v| v))
        .build();
}

genet_dissectors!(TcpStreamDissector {});
