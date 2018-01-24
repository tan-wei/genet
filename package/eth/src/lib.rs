extern crate libc;
extern crate byteorder;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate plugkit;

use std::collections::HashMap;
use std::io::{Cursor,Error,ErrorKind};
use byteorder::{BigEndian};
use plugkit::token;
use plugkit::reader::ByteReader;
use plugkit::layer::{Layer,Confidence};
use plugkit::context::Context;
use plugkit::worker::Worker;
use plugkit::variant::Value;
use plugkit::attr::ResultValue;

pub mod file;

lazy_static! {
    static ref ETH_TOKEN: token::Token = token::get("eth");
    static ref SRC_TOKEN: token::Token = token::get("eth.src");
    static ref DST_TOKEN: token::Token = token::get("eth.dst");
    static ref LEN_TOKEN: token::Token = token::get("eth.len");
    static ref TYPE_TOKEN: token::Token = token::get("eth.type");
    static ref ENUM_TOKEN: token::Token = token::get("@enum");
    static ref MAC_TOKEN: token::Token = token::get("@eth:mac");
    static ref NOVALUE_TOKEN: token::Token = token::get("@novalue");

    static ref TYPEMAP: HashMap<u32, (token::Token, token::Token)> = {
        let mut m = HashMap::new();
        m.insert(0x0800, (token::get("[ipv4]"), token::get("eth.type.ipv4")));
        m.insert(0x86DD, (token::get("[ipv6]"), token::get("eth.type.ipv6")));
        m
 };
}

struct ETHWorker {}

impl Worker for ETHWorker {
    fn analyze(&self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
        let (slice, range) = {
            let payload = layer.payloads().next().ok_or(Error::new(ErrorKind::Other, "no payload"))?;
            let slice = payload.slices().next().ok_or(Error::new(ErrorKind::Other, "no slice"))?;
            (slice, payload.range())
        };

        let mut rdr = Cursor::new(slice);

        let child = layer.add_layer(ctx, *ETH_TOKEN);
        child.set_confidence(Confidence::Error);
        child.add_tag(*ETH_TOKEN);
        child.set_range(range);
        {
            let attr = child.add_attr(ctx, *SRC_TOKEN);
            attr.set_typ(*MAC_TOKEN);
            attr.set_result(ByteReader::read_slice(&mut rdr, 6))?;
        }
        {
            let attr = child.add_attr(ctx, *DST_TOKEN);
            attr.set_typ(*MAC_TOKEN);
            attr.set_result(ByteReader::read_slice(&mut rdr, 6))?;
        }

        let (typ, range) = ByteReader::read_u16::<BigEndian>(&mut rdr)?;
        if typ <= 1500 {
            let attr = child.add_attr(ctx, *LEN_TOKEN);
            attr.set(&(typ as u32));
            attr.set_range(&(12..14));
        } else {
            {
                let attr = child.add_attr(ctx, *TYPE_TOKEN);
                attr.set(&(typ as u32));
                attr.set_typ(*ENUM_TOKEN);
                attr.set_range(&range);
            }
            if let Some(item) = TYPEMAP.get(&typ) {
                let &(tag, id) = item;
                child.add_tag(tag);
                let attr = child.add_attr(ctx, id);
                attr.set(&true);
                attr.set_typ(*NOVALUE_TOKEN);
                attr.set_range(&range);
            }
        }
        {
            let (data, range) = ByteReader::read_slice_to_end(&mut rdr)?;
            let payload = child.add_payload(ctx);
            payload.add_slice(data);
            payload.set_range(&range);
        }

        child.set_confidence(Confidence::Exact);
        Ok(())
    }
}

plugkit_module! ({});
plugkit_api_layer_hints! (token::get("[eth]"));
plugkit_api_worker! (ETHWorker, ETHWorker{});
