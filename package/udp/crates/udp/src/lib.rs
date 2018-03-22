extern crate byteorder;
extern crate libc;

#[macro_use]
extern crate plugkit;

use std::io::{Cursor, Error, ErrorKind};
use byteorder::BigEndian;
use plugkit::reader::ByteReader;
use plugkit::layer::Layer;
use plugkit::context::Context;
use plugkit::worker::Worker;
use plugkit::variant::Value;
use plugkit::attr::ResultValue;

struct UDPWorker {}

impl Worker for UDPWorker {
    fn analyze(&mut self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
        let (slice, payload_range) = {
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

        let child = layer.add_layer(ctx, token!("udp"));
        child.add_tag(ctx, token!("udp"));
        child.set_range(&payload_range);

        (|| -> Result<(), Error> {
            let mut rdr = Cursor::new(slice);
            {
                child.add_attr_alias(ctx, token!("_.src"), token!("udp.src"));
                let attr = child.add_attr(ctx, token!("udp.src"));
                attr.set_typ(token!("@udp:port"));
                attr.set_with_range(&ByteReader::read_u16::<BigEndian>(&mut rdr)?);
            }
            {
                child.add_attr_alias(ctx, token!("_.dst"), token!("udp.dst"));
                let attr = child.add_attr(ctx, token!("udp.dst"));
                attr.set_typ(token!("@udp:port"));
                attr.set_with_range(&ByteReader::read_u16::<BigEndian>(&mut rdr)?);
            }
            let (len, len_range) = ByteReader::read_u16::<BigEndian>(&mut rdr)?;
            {
                let attr = child.add_attr(ctx, token!("udp.length"));
                attr.set(&len);
                attr.set_range(&len_range);
            }
            {
                let attr = child.add_attr(ctx, token!("udp.checksum"));
                attr.set_with_range(&ByteReader::read_u16::<BigEndian>(&mut rdr)?);
            }
            let (data, range) = ByteReader::read_slice(&mut rdr, len as usize - 8)?;
            let payload = child.add_payload(ctx);
            let offset = payload_range.start;
            payload.add_slice(data);
            payload.set_range(&(range.start + offset..range.end + offset));

            Ok(())
        })().or_else(|_| {
            child.add_error(ctx, token!("!out-of-bounds"), "");
            Ok(())
        })
    }
}

plugkit_module!({});
plugkit_api_layer_hints!(token!("[udp]"));
plugkit_api_worker!(UDPWorker, UDPWorker {});
