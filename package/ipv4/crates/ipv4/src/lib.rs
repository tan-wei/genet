extern crate byteorder;
extern crate libc;

#[macro_use]
extern crate plugkit;

use std::io::{Cursor, Error, ErrorKind};
use byteorder::BigEndian;
use plugkit::reader::ByteReader;
use plugkit::layer::{Confidence, Layer};
use plugkit::context::Context;
use plugkit::worker::Worker;
use plugkit::variant::Value;
use plugkit::token::Token;
use plugkit::attr::ResultValue;

fn get_protocol(val: u32) -> Option<(Token, Token)> {
    match val {
        0x01 => Some((token!("[icmp]"), token!("ipv4.protocol.icmp"))),
        0x02 => Some((token!("[igmp]"), token!("ipv4.protocol.igmp"))),
        0x06 => Some((token!("[tcp]"), token!("ipv4.protocol.tcp"))),
        0x11 => Some((token!("[udp]"), token!("ipv4.protocol.udp"))),
        _ => None,
    }
}

struct IPv4Worker {}

impl Worker for IPv4Worker {
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

        let child = layer.add_layer(ctx, token!("ipv4"));
        child.set_confidence(Confidence::Error);
        child.add_tag(ctx, token!("ipv4"));
        child.set_range(&payload_range);

        let mut rdr = Cursor::new(slice);

        let result = {
            let attr = child.add_attr(ctx, token!("ipv4.version"));
            attr.set_result_map(ByteReader::read_u8(&mut rdr), |v| v >> 4)
        };
        {
            let attr = child.add_attr(ctx, token!("ipv4.headerLength"));
            attr.set_result_map(result, |v| v & 0b00001111)?;
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.type"));
            attr.set_result(ByteReader::read_u8(&mut rdr))?;
        }

        let (total_length, range) = ByteReader::read_u16::<BigEndian>(&mut rdr)?;
        {
            let attr = child.add_attr(ctx, token!("ipv4.totalLength"));
            attr.set(&total_length);
            attr.set_range(&range);
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.id"));
            attr.set_result(ByteReader::read_u16::<BigEndian>(&mut rdr))?;
        }

        let (flag_and_offset, range) = ByteReader::read_u8(&mut rdr)?;
        let flag = (flag_and_offset >> 5) & 0b00000111;
        {
            let attr = child.add_attr(ctx, token!("ipv4.flags"));
            attr.set_typ(token!("@flags"));
            attr.set(&flag);
            attr.set_range(&range);
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.flags.reserved"));
            attr.set(&(flag & 0x01 != 0));
            attr.set_range(&range);
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.flags.dontFragment"));
            attr.set(&(flag & 0x02 != 0));
            attr.set_range(&range);
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.flags.moreFragments"));
            attr.set(&(flag & 0x04 != 0));
            attr.set_range(&range);
        }

        let (fg_offset, _) = ByteReader::read_u8(&mut rdr)?;
        {
            let attr = child.add_attr(ctx, token!("ipv4.fragmentOffset"));
            let offset = ((flag_and_offset & 0b00011111) << 8) | fg_offset;
            attr.set(&offset);
            attr.set_range(&(6 .. 8));
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.ttl"));
            attr.set_result(ByteReader::read_u8(&mut rdr))?;
        }

        let (protocol, range) = ByteReader::read_u8(&mut rdr)?;
        {
            let attr = child.add_attr(ctx, token!("ipv4.protocol"));
            attr.set_typ(token!("@enum"));
            attr.set(&protocol);
            attr.set_range(&range);
        }
        if let Some(item) = get_protocol(protocol) {
            let (tag, id) = item;
            child.add_tag(ctx, tag);
            let attr = child.add_attr(ctx, id);
            attr.set(&true);
            attr.set_typ(token!("@novalue"));
            attr.set_range(&range);
        }

        {
            let attr = child.add_attr(ctx, token!("ipv4.checksum"));
            attr.set_result(ByteReader::read_u16::<BigEndian>(&mut rdr))?;
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.src"));
            attr.set_typ(token!("@ipv4:addr"));
            attr.set_result(ByteReader::read_slice(&mut rdr, 4))?;
        }
        {
            let attr = child.add_attr(ctx, token!("ipv4.dst"));
            attr.set_typ(token!("@ipv4:addr"));
            attr.set_result(ByteReader::read_slice(&mut rdr, 4))?;
        }
        {
            let (data, range) = ByteReader::read_slice_to_end(&mut rdr)?;
            let payload = child.add_payload(ctx);
            let offset = payload_range.start;
            payload.add_slice(data);
            payload.set_range(&(range.start + offset..range.end + offset));
        }

        child.set_confidence(Confidence::Exact);
        Ok(())
    }
}

plugkit_module!({});
plugkit_api_layer_hints!(token!("[ipv4]"));
plugkit_api_worker!(IPv4Worker, IPv4Worker {});
