#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate maplit;

use genet_sdk::prelude::*;
use std::collections::HashMap;

struct NtpWorker {}

impl Worker for NtpWorker {
    fn analyze(
        &mut self,
        _ctx: &mut Context,
        stack: &LayerStack,
        parent: &mut Layer,
    ) -> Result<Status> {
        if parent.id() != token!("udp") {
            return Ok(Status::Skip);
        }
        if let Some(payload) = parent.payloads().iter().next() {
            let parent_src: u16 = stack
                .attr(token!("udp.src"))
                .unwrap()
                .try_get(parent)?
                .try_into()?;

            let parent_dst: u16 = stack
                .attr(token!("udp.dst"))
                .unwrap()
                .try_get(parent)?
                .try_into()?;

            if parent_src != 123 && parent_dst != 123 {
                return Ok(Status::Skip);
            }

            let mut layer = Layer::new(&NTP_CLASS, payload.data());

            let leap_attr = Attr::new(&LEAP_ATTR, 0..1);
            let leap_type = leap_attr.try_get(&layer)?.try_into()?;

            let leap = LEAP_MAP.get(&leap_type);
            if let Some(attr) = leap {
                layer.add_attr(Attr::new(attr, 0..1));
            }

            let mode_attr = Attr::new(&MODE_ATTR, 0..1);
            let mode_type = mode_attr.try_get(&layer)?.try_into()?;

            let mode = MODE_MAP.get(&mode_type);
            if let Some(attr) = mode {
                layer.add_attr(Attr::new(attr, 0..1));
            }

            let stratum_attr = Attr::new(&STRATUM_ATTR, 1..2);
            let stratum: u8 = stratum_attr.try_get(&layer)?.try_into()?;

            layer.add_attr(if stratum >= 2 {
                Attr::new(&ID_IP_ATTR, 12..16)
            } else {
                Attr::new(&ID_ATTR, 12..16)
            });

            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct NtpDissector {}

impl Dissector for NtpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "parallel" {
            Some(Box::new(NtpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref NTP_CLASS: Ptr<LayerClass> = LayerBuilder::new("ntp")
        .header(Attr::new(&LEAP_ATTR, 0..1))
        .header(Attr::new(&VERSION_ATTR, 0..1))
        .header(Attr::new(&MODE_ATTR, 0..1))
        .header(Attr::new(&STRATUM_ATTR, 1..2))
        .header(Attr::new(&POLL_ATTR, 2..3))
        .header(Attr::new(&PRECISION_ATTR, 3..4))
        .header(Attr::new(&RDELAY_ATTR, 4..8))
        .header(Attr::new(&RDELAY_SEC_ATTR, 4..6))
        .header(Attr::new(&RDELAY_FRA_ATTR, 6..8))
        .header(Attr::new(&RDISP_ATTR, 8..12))
        .header(Attr::new(&RDISP_SEC_ATTR, 8..10))
        .header(Attr::new(&RDISP_FRA_ATTR, 10..12))
        .header(Attr::new(&REFTS_ATTR, 16..24))
        .header(Attr::new(&REFTS_SEC_ATTR, 16..20))
        .header(Attr::new(&REFTS_FRA_ATTR, 20..24))
        .header(Attr::new(&ORITS_ATTR, 24..32))
        .header(Attr::new(&ORITS_SEC_ATTR, 24..28))
        .header(Attr::new(&ORITS_FRA_ATTR, 28..32))
        .header(Attr::new(&RECTS_ATTR, 32..40))
        .header(Attr::new(&RECTS_SEC_ATTR, 32..36))
        .header(Attr::new(&RECTS_FRA_ATTR, 36..40))
        .header(Attr::new(&TRATS_ATTR, 40..48))
        .header(Attr::new(&TRATS_SEC_ATTR, 40..44))
        .header(Attr::new(&TRATS_FRA_ATTR, 44..48))
        .build();
    static ref LEAP_ATTR: AttrClass = AttrBuilder::new("ntp.leapIndicator")
        .typ("@enum")
        .decoder(decoder::UInt8().map(|v| v >> 6))
        .build();
    static ref VERSION_ATTR: AttrClass = AttrBuilder::new("ntp.version")
        .decoder(decoder::UInt8().map(|v| (v >> 3) & 0b111))
        .build();
    static ref MODE_ATTR: AttrClass = AttrBuilder::new("ntp.mode")
        .typ("@enum")
        .decoder(decoder::UInt8().map(|v| v & 0b111))
        .build();
    static ref STRATUM_ATTR: AttrClass = AttrBuilder::new("ntp.stratum")
        .decoder(decoder::UInt8())
        .build();
    static ref POLL_ATTR: AttrClass = AttrBuilder::new("ntp.pollInterval")
        .decoder(decoder::UInt8())
        .build();
    static ref PRECISION_ATTR: AttrClass = AttrBuilder::new("ntp.precision")
        .decoder(decoder::UInt8())
        .build();
    static ref RDELAY_ATTR: AttrClass = AttrBuilder::new("ntp.rootDelay")
        .decoder(decoder::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)))
        .build();
    static ref RDELAY_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.rootDelay.seconds")
        .decoder(decoder::UInt16BE())
        .build();
    static ref RDELAY_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.rootDelay.fraction")
        .decoder(decoder::UInt16BE())
        .build();
    static ref RDISP_ATTR: AttrClass = AttrBuilder::new("ntp.rootDispersion")
        .decoder(decoder::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)))
        .build();
    static ref RDISP_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.rootDispersion.seconds")
        .decoder(decoder::UInt16BE())
        .build();
    static ref RDISP_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.rootDispersion.fraction")
        .decoder(decoder::UInt16BE())
        .build();
    static ref ID_ATTR: AttrClass = AttrBuilder::new("ntp.identifier")
        .decoder(decoder::ByteSlice())
        .build();
    static ref ID_IP_ATTR: AttrClass = AttrBuilder::new("ntp.identifier")
        .typ("@ipv4:addr")
        .decoder(decoder::ByteSlice())
        .build();
    static ref REFTS_ATTR: AttrClass = AttrBuilder::new("ntp.referenceTs")
        .typ("@ntp:time")
        .decoder(
            decoder::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        )
        .build();
    static ref REFTS_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.referenceTs.seconds")
        .decoder(decoder::UInt32BE())
        .build();
    static ref REFTS_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.referenceTs.fraction")
        .decoder(decoder::UInt32BE())
        .build();
    static ref ORITS_ATTR: AttrClass = AttrBuilder::new("ntp.originateTs")
        .typ("@ntp:time")
        .decoder(
            decoder::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        )
        .build();
    static ref ORITS_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.originateTs.seconds")
        .decoder(decoder::UInt32BE())
        .build();
    static ref ORITS_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.originateTs.fraction")
        .decoder(decoder::UInt32BE())
        .build();
    static ref RECTS_ATTR: AttrClass = AttrBuilder::new("ntp.receiveTs")
        .typ("@ntp:time")
        .decoder(
            decoder::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        )
        .build();
    static ref RECTS_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.receiveTs.seconds")
        .decoder(decoder::UInt32BE())
        .build();
    static ref RECTS_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.receiveTs.fraction")
        .decoder(decoder::UInt32BE())
        .build();
    static ref TRATS_ATTR: AttrClass = AttrBuilder::new("ntp.transmitTs")
        .typ("@ntp:time")
        .decoder(
            decoder::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        )
        .build();
    static ref TRATS_SEC_ATTR: AttrClass = AttrBuilder::new("ntp.transmitTs.seconds")
        .decoder(decoder::UInt32BE())
        .build();
    static ref TRATS_FRA_ATTR: AttrClass = AttrBuilder::new("ntp.transmitTs.fraction")
        .decoder(decoder::UInt32BE())
        .build();
    static ref LEAP_MAP: HashMap<u64, AttrClass> = hashmap!{
        0 => AttrBuilder::new("ntp.leapIndicator.noWarning").typ("@novalue").decoder(decoder::Const(true)).build(),
        1 => AttrBuilder::new("ntp.leapIndicator.sec61").typ("@novalue").decoder(decoder::Const(true)).build(),
        2 => AttrBuilder::new("ntp.leapIndicator.sec59").typ("@novalue").decoder(decoder::Const(true)).build(),
        3 => AttrBuilder::new("ntp.leapIndicator.unknown").typ("@novalue").decoder(decoder::Const(true)).build(),
    };
    static ref MODE_MAP: HashMap<u64, AttrClass> = hashmap!{
        0 => AttrBuilder::new("ntp.mode.reserved").typ("@novalue").decoder(decoder::Const(true)).build(),
        1 => AttrBuilder::new("ntp.mode.symmetricActive").typ("@novalue").decoder(decoder::Const(true)).build(),
        2 => AttrBuilder::new("ntp.mode.symmetricPassive").typ("@novalue").decoder(decoder::Const(true)).build(),
        3 => AttrBuilder::new("ntp.mode.client").typ("@novalue").decoder(decoder::Const(true)).build(),
        4 => AttrBuilder::new("ntp.mode.server").typ("@novalue").decoder(decoder::Const(true)).build(),
        5 => AttrBuilder::new("ntp.mode.broadcast").typ("@novalue").decoder(decoder::Const(true)).build(),
        6 => AttrBuilder::new("ntp.mode.controlMessage").typ("@novalue").decoder(decoder::Const(true)).build(),
        7 => AttrBuilder::new("ntp.mode.reservedForPrivate").typ("@novalue").decoder(decoder::Const(true)).build(),
    };
}

genet_dissectors!(NtpDissector {});
