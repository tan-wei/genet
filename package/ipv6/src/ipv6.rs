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
        0x01 => Some((token!("[icmp]"), token!("ipv6.protocol.icmp"))),
        0x02 => Some((token!("[igmp]"), token!("ipv6.protocol.igmp"))),
        0x06 => Some((token!("[tcp]"), token!("ipv6.protocol.tcp"))),
        0x11 => Some((token!("[udp]"), token!("ipv6.protocol.udp"))),
        _ => None,
    }
}

struct IPv6Worker {}

impl Worker for IPv6Worker {
    fn analyze(&self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
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

        let child = layer.add_layer(ctx, token!("ipv6"));
        child.set_confidence(Confidence::Error);
        child.add_tag(token!("ipv6"));
        child.set_range(&payload_range);

        let mut rdr = Cursor::new(slice);
        let (header, _) = ByteReader::read_u8(&mut rdr)?;
        let (header2, _) = ByteReader::read_u8(&mut rdr)?;

        let version = header >> 4;
        let traffic_class = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);

        {
            let attr = child.add_attr(ctx, token!("ipv6.version"));
            attr.set(&version);
            attr.set_range(&(0..1));
        }
        {
            let attr = child.add_attr(ctx, token!("ipv6.trafficClass"));
            attr.set(&traffic_class);
            attr.set_range(&(0..2));
        }

        let (flow_level, _) = ByteReader::read_u16::<BigEndian>(&mut rdr)?;
        {
            let attr = child.add_attr(ctx, token!("ipv6.flowLevel"));
            attr.set(&(flow_level | ((header2 & 0b00001111) << 16)));
            attr.set_range(&(1..4));
        }
        {
            let attr = child.add_attr(ctx, token!("ipv6.payloadLength"));
            attr.set_result(ByteReader::read_u16::<BigEndian>(&mut rdr))?;
        }

        let (mut next_header, mut next_range) = ByteReader::read_u8(&mut rdr)?;
        {
            let attr = child.add_attr(ctx, token!("ipv6.nextHeader"));
            attr.set(&next_header);
            attr.set_range(&next_range);
        }
        {
            let attr = child.add_attr(ctx, token!("ipv6.hopLimit"));
            attr.set_result(ByteReader::read_u8(&mut rdr))?;
        }

        {
            let attr = child.add_attr(ctx, token!("ipv6.src"));
            attr.set_typ(token!("@ipv6:addr"));
            attr.set_result(ByteReader::read_slice(&mut rdr, 16))?;
        }
        {
            let attr = child.add_attr(ctx, token!("ipv6.dst"));
            attr.set_typ(token!("@ipv6:addr"));
            attr.set_result(ByteReader::read_slice(&mut rdr, 16))?;
        }

        loop {
            match next_header {
                // Hop-by-Hop Options, Destination Options
                0 | 60 => {
                    let (h, r) = ByteReader::read_u8(&mut rdr)?;
                    next_header = h;
                    next_range = r;
                    let (ext_len, _) = ByteReader::read_u8(&mut rdr)?;
                    let byte_len = (ext_len + 1) * 8;
                    ByteReader::read_slice(&mut rdr, byte_len as usize)?;
                },
                // TODO:
                // case 43  # Routing
                // case 44  # Fragment
                // case 51  # Authentication Header
                // case 50  # Encapsulating Security Payload
                // case 135 # Mobility
                // No Next Header
                59 => break,
                _ => break
            }
        }

        let protocol = next_header;
        let range = next_range;
        {
            let attr = child.add_attr(ctx, token!("ipv6.protocol"));
            attr.set_typ(token!("@enum"));
            attr.set(&protocol);
            attr.set_range(&range);
        }
        if let Some(item) = get_protocol(protocol) {
            let (tag, id) = item;
            child.add_tag(tag);
            let attr = child.add_attr(ctx, id);
            attr.set(&true);
            attr.set_typ(token!("@novalue"));
            attr.set_range(&range);
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
plugkit_api_layer_hints!(token!("[ipv6]"));
plugkit_api_worker!(IPv6Worker, IPv6Worker {});
