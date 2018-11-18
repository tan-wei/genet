extern crate genet_sdk;

use genet_sdk::{decoder::*, prelude::*};
use std::collections::{BTreeMap, HashMap};

#[derive(Debug)]
struct Stream {
    pub id: u64,
    pub seq: i64,
    pub len: usize,
    offset: usize,
    slices: BTreeMap<usize, ByteSlice>,
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

    fn put(&mut self, start: usize, data: ByteSlice) {
        if data.len() > 0 {
            self.slices.insert(start, data);
            let mut end = self.offset;
            for (start, slice) in self.slices.iter_mut() {
                if *start < end {}
                end = *start + slice.len();
            }
        }
    }

    fn fetch(&mut self) -> impl Iterator<Item = ByteSlice> {
        let mut slices = Vec::new();
        loop {
            let pos;
            if let Some((key, value)) = self.slices.iter().next() {
                pos = *key;
                if *key == self.offset {
                    self.offset += value.len();
                    slices.push(value.clone());
                } else {
                    break;
                }
            } else {
                break;
            }
            self.slices.remove(&pos);
        }
        slices.into_iter()
    }
}

struct TcpStreamWorker {
    map: HashMap<(ByteSlice, ByteSlice, u32, u32), Stream>,
}

impl TcpStreamWorker {
    fn new() -> TcpStreamWorker {
        TcpStreamWorker {
            map: HashMap::new(),
        }
    }
}

impl Worker for TcpStreamWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        if parent.id() == token!("tcp") {
            let slice: ByteSlice = parent
                .payloads()
                .find(|p| p.id() == token!("@data:tcp"))
                .unwrap()
                .data();

            let stream_id = {
                let parent_src: ByteSlice = stack
                    .attr(token!("_.src"))
                    .unwrap()
                    .try_get()?
                    .try_into()?;
                let parent_dst: ByteSlice = stack
                    .attr(token!("_.dst"))
                    .unwrap()
                    .try_get()?
                    .try_into()?;
                let src: u32 = parent
                    .attr(token!("tcp.src"))
                    .unwrap()
                    .try_get()?
                    .try_into()?;
                let dst: u32 = parent
                    .attr(token!("tcp.dst"))
                    .unwrap()
                    .try_get()?
                    .try_into()?;
                (parent_src, parent_dst, src, dst)
            };

            let id = self.map.len();
            let stream = self
                .map
                .entry(stream_id)
                .or_insert_with(|| Stream::new(id as u64));

            let seq: u32 = parent
                .attr(token!("tcp.seq"))
                .unwrap()
                .try_get()?
                .try_into()?;
            let window: u16 = parent
                .attr(token!("tcp.window"))
                .unwrap()
                .try_get()?
                .try_into()?;
            let flags: u8 = parent
                .attr(token!("tcp.flags"))
                .unwrap()
                .try_get()?
                .try_into()?;

            let syn = (flags & (0x1 << 1)) != 0;
            if syn {
                if stream.seq < 0 {
                    let offset = stream.len;
                    stream.seq = seq as i64;
                    stream.len += slice.len();
                    stream.put(offset, slice);
                }
            } else if stream.seq >= 0 {
                if slice.len() > 0 {
                    if seq >= stream.seq as u32 {
                        let offset = stream.len + (seq - stream.seq as u32) as usize;
                        stream.seq = seq as i64;
                        stream.len += slice.len();
                        stream.put(offset, slice);
                    } else if stream.seq - seq as i64 > window as i64 {
                        let offset = stream.len
                            + ((std::u32::MAX as u32 - stream.seq as u32) + seq) as usize;
                        stream.seq = seq as i64;
                        stream.len += slice.len();
                        stream.put(offset, slice);
                    }
                } else if (stream.seq + 1) % std::u32::MAX as i64 == seq as i64 {
                    stream.seq = seq as i64;
                }
            }

            let payloads = stream.fetch();
            for payload in payloads {
                parent.add_payload(Payload::new(payload, "@stream:tcp"));
            }

            Ok(Status::Done)
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct TcpStreamDecoder {}

impl Decoder for TcpStreamDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(TcpStreamWorker::new())
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::SerialSync,
            ..Metadata::default()
        }
    }
}

genet_decoders!(TcpStreamDecoder {});
