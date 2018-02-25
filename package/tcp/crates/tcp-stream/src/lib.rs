extern crate byteorder;
extern crate libc;

#[macro_use]
extern crate plugkit;

use std::collections::{HashMap, BTreeMap};
use std::io::{Error, ErrorKind};
use plugkit::layer::{Layer};
use plugkit::context::Context;
use plugkit::worker::Worker;
use plugkit::variant::Value;

#[derive(Debug)]
struct Stream {
  pub id: u32,
  pub seq: i64,
  pub len: usize,
  offset: usize,
  slices: BTreeMap<usize, &'static[u8]>
}

impl Stream {
    fn new(id: u32) -> Stream {
        return Stream {
            id: id,
            seq: -1,
            len: 0,
            offset: 0,
            slices: BTreeMap::new()
        }
    }

    fn put(&mut self, start: usize, data: &'static[u8]) {
        if data.len() > 0 {
            self.slices.insert(start, data);
            let mut end = self.offset;
            for (start, slice) in self.slices.iter_mut() {
                if *start < end {

                }
                end = *start + slice.len();
            }
        }
    }

    fn fetch(&mut self) -> Box<[&'static[u8]]> {
        let mut slices = Vec::<&'static[u8]>::new();
        loop {
            let pos;
            if let Some((key, value)) = self.slices.iter().next() {
                pos = *key;
                if *key == self.offset {
                    self.offset += value.len();
                    slices.push(value);
                } else {
                    break
                }
            } else {
                break
            }
            self.slices.remove(&pos);
        }
        slices.into_boxed_slice()
    }
}

struct TCPStreamWorker {
    map: HashMap<(&'static[u8],&'static[u8],u32,u32), Stream>
}

impl TCPStreamWorker {
    fn new() -> TCPStreamWorker {
        return TCPStreamWorker {
            map: HashMap::new()
        }
    }
}

impl Worker for TCPStreamWorker {
    fn analyze(&mut self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
        let (slice, _) = {
            let payload = layer
                .payloads()
                .next()
                .ok_or(Error::new(ErrorKind::Other, "no payload"))?;
            let slice = payload
                .slices()
                .next()
                .ok_or(Error::new(ErrorKind::Other, "no slice"))?;

            (slice, payload.range())
        };

        let stream_id = {
            let parent = layer.parent().unwrap();
            let parent_src : &[u8] = parent.attr(token!("_.src")).unwrap().get();
            let parent_dst : &[u8] = parent.attr(token!("_.dst")).unwrap().get();
            let src : u32 = layer.attr(token!("tcp.src")).unwrap().get();
            let dst : u32 = layer.attr(token!("tcp.dst")).unwrap().get();
            (parent_src, parent_dst, src, dst)
        };

        let id = (self.map.len() << 8) | layer.worker() as usize;
        let stream = self.map.entry(stream_id).or_insert_with(|| Stream::new(id as u32));
        {
            let attr = layer.add_attr(ctx, token!("_.streamId"));
            attr.set(&stream.id);
        }

        let (seq, window, flags) = {
            let seq : u32 = layer.attr(token!("tcp.seq")).unwrap().get();
            let window : u16 = layer.attr(token!("tcp.window")).unwrap().get();
            let flags : u8 = layer.attr(token!("tcp.flags")).unwrap().get();
            (seq, window, flags)
        };

        let syn = (flags & (0x1 << 1)) != 0;
        if syn {
            if stream.seq < 0 {
                let offset = stream.len;
                stream.seq = seq as i64;
                stream.put(offset, slice);
                stream.len += slice.len();
            }
        } else if stream.seq >= 0 {
            if slice.len() > 0 {
                if seq >= stream.seq as u32 {
                    let offset = stream.len + (seq - stream.seq as u32) as usize;
                    stream.seq = seq as i64;
                    stream.put(offset, slice);
                    stream.len += slice.len();
                } else if stream.seq - seq as i64 > window as i64 {
                    let offset = stream.len + ((std::u32::MAX as u32 - stream.seq as u32) + seq) as usize;
                    stream.seq = seq as i64;
                    stream.put(offset, slice);
                    stream.len += slice.len();
                }
            } else if (stream.seq + 1) % std::u32::MAX as i64 == seq as i64 {
                stream.seq = seq as i64;
            }
        }

        let payloads = stream.fetch();
        if payloads.len() > 0 {
            let chunk = layer.add_payload(ctx);
            chunk.set_typ(token!("@stream"));
            for s in &*payloads {
                chunk.add_slice(s);
            }
        }

        let child = layer.add_sublayer(ctx, token!("tcp-stream"));
        child.add_tag(ctx, token!("tcp-stream"));
        Ok(())
    }
}

plugkit_module!({});
plugkit_api_layer_hints!(token!("tcp"));
plugkit_api_worker!(TCPStreamWorker, TCPStreamWorker::new());
