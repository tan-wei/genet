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
            let leap_type = LEAP_ATTR_HEADER.try_get(&layer)?.try_into()?;

            let leap = LEAP_MAP.get(&leap_type);
            if let Some(attr) = leap {
                layer.add_attr(Attr::new(attr, 0..1));
            }

            let mode_type = MODE_ATTR_HEADER.try_get(&layer)?.try_into()?;

            let mode = MODE_MAP.get(&mode_type);
            if let Some(attr) = mode {
                layer.add_attr(Attr::new(attr, 0..1));
            }

            let stratum: u8 = STRATUM_ATTR_HEADER.try_get(&layer)?.try_into()?;
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
    static ref LEAP_ATTR_HEADER: Attr = Attr::new(&LEAP_ATTR, 0..1);
    static ref MODE_ATTR_HEADER: Attr = Attr::new(&MODE_ATTR, 0..1);
    static ref STRATUM_ATTR_HEADER: Attr = Attr::new(&STRATUM_ATTR, 1..2);
    static ref NTP_CLASS: LayerClass = LayerClass::builder("ntp")
        .header(&LEAP_ATTR_HEADER)
        .header(Attr::new(&VERSION_ATTR, 0..1))
        .header(&MODE_ATTR_HEADER)
        .header(&STRATUM_ATTR_HEADER)
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
}

lazy_static! {
    static ref LEAP_ATTR: AttrClass = AttrClass::builder("ntp.leapIndicator")
        .typ("@enum")
        .cast(cast::UInt8().map(|v| v >> 6))
        .build();
    static ref VERSION_ATTR: AttrClass = AttrClass::builder("ntp.version")
        .cast(cast::UInt8().map(|v| (v >> 3) & 0b111))
        .build();
    static ref MODE_ATTR: AttrClass = AttrClass::builder("ntp.mode")
        .typ("@enum")
        .cast(cast::UInt8().map(|v| v & 0b111))
        .build();
    static ref STRATUM_ATTR: AttrClass = AttrClass::builder("ntp.stratum")
        .cast(cast::UInt8())
        .build();
    static ref POLL_ATTR: AttrClass = AttrClass::builder("ntp.pollInterval")
        .cast(cast::UInt8())
        .build();
    static ref PRECISION_ATTR: AttrClass = AttrClass::builder("ntp.precision")
        .cast(cast::UInt8())
        .build();
    static ref RDELAY_ATTR: AttrClass = AttrClass::builder("ntp.rootDelay")
        .cast(cast::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)))
        .build();
    static ref RDELAY_SEC_ATTR: AttrClass = AttrClass::builder("ntp.rootDelay.seconds")
        .cast(cast::UInt16BE())
        .build();
    static ref RDELAY_FRA_ATTR: AttrClass = AttrClass::builder("ntp.rootDelay.fraction")
        .cast(cast::UInt16BE())
        .build();
    static ref RDISP_ATTR: AttrClass = AttrClass::builder("ntp.rootDispersion")
        .cast(cast::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)))
        .build();
    static ref RDISP_SEC_ATTR: AttrClass = AttrClass::builder("ntp.rootDispersion.seconds")
        .cast(cast::UInt16BE())
        .build();
    static ref RDISP_FRA_ATTR: AttrClass = AttrClass::builder("ntp.rootDispersion.fraction")
        .cast(cast::UInt16BE())
        .build();
    static ref ID_ATTR: AttrClass = AttrClass::builder("ntp.identifier")
        .cast(cast::ByteSlice())
        .build();
    static ref ID_IP_ATTR: AttrClass = AttrClass::builder("ntp.identifier")
        .typ("@ipv4:addr")
        .cast(cast::ByteSlice())
        .build();
    static ref REFTS_ATTR: AttrClass = AttrClass::builder("ntp.referenceTs")
        .typ("@ntp:time")
        .cast(
            cast::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        ).build();
    static ref REFTS_SEC_ATTR: AttrClass = AttrClass::builder("ntp.referenceTs.seconds")
        .cast(cast::UInt32BE())
        .build();
    static ref REFTS_FRA_ATTR: AttrClass = AttrClass::builder("ntp.referenceTs.fraction")
        .cast(cast::UInt32BE())
        .build();
    static ref ORITS_ATTR: AttrClass = AttrClass::builder("ntp.originateTs")
        .typ("@ntp:time")
        .cast(
            cast::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        ).build();
    static ref ORITS_SEC_ATTR: AttrClass = AttrClass::builder("ntp.originateTs.seconds")
        .cast(cast::UInt32BE())
        .build();
    static ref ORITS_FRA_ATTR: AttrClass = AttrClass::builder("ntp.originateTs.fraction")
        .cast(cast::UInt32BE())
        .build();
    static ref RECTS_ATTR: AttrClass = AttrClass::builder("ntp.receiveTs")
        .typ("@ntp:time")
        .cast(
            cast::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        ).build();
    static ref RECTS_SEC_ATTR: AttrClass = AttrClass::builder("ntp.receiveTs.seconds")
        .cast(cast::UInt32BE())
        .build();
    static ref RECTS_FRA_ATTR: AttrClass = AttrClass::builder("ntp.receiveTs.fraction")
        .cast(cast::UInt32BE())
        .build();
    static ref TRATS_ATTR: AttrClass = AttrClass::builder("ntp.transmitTs")
        .typ("@ntp:time")
        .cast(
            cast::UInt64BE()
                .map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64))
        ).build();
    static ref TRATS_SEC_ATTR: AttrClass = AttrClass::builder("ntp.transmitTs.seconds")
        .cast(cast::UInt32BE())
        .build();
    static ref TRATS_FRA_ATTR: AttrClass = AttrClass::builder("ntp.transmitTs.fraction")
        .cast(cast::UInt32BE())
        .build();
}

lazy_static! {
    static ref LEAP_MAP: HashMap<u64, AttrClass> = hashmap!{
        0 => AttrClass::builder("ntp.leapIndicator.noWarning").typ("@novalue").cast(cast::Const(true)).build(),
        1 => AttrClass::builder("ntp.leapIndicator.sec61").typ("@novalue").cast(cast::Const(true)).build(),
        2 => AttrClass::builder("ntp.leapIndicator.sec59").typ("@novalue").cast(cast::Const(true)).build(),
        3 => AttrClass::builder("ntp.leapIndicator.unknown").typ("@novalue").cast(cast::Const(true)).build(),
    };
    static ref MODE_MAP: HashMap<u64, AttrClass> = hashmap!{
        0 => AttrClass::builder("ntp.mode.reserved").typ("@novalue").cast(cast::Const(true)).build(),
        1 => AttrClass::builder("ntp.mode.symmetricActive").typ("@novalue").cast(cast::Const(true)).build(),
        2 => AttrClass::builder("ntp.mode.symmetricPassive").typ("@novalue").cast(cast::Const(true)).build(),
        3 => AttrClass::builder("ntp.mode.client").typ("@novalue").cast(cast::Const(true)).build(),
        4 => AttrClass::builder("ntp.mode.server").typ("@novalue").cast(cast::Const(true)).build(),
        5 => AttrClass::builder("ntp.mode.broadcast").typ("@novalue").cast(cast::Const(true)).build(),
        6 => AttrClass::builder("ntp.mode.controlMessage").typ("@novalue").cast(cast::Const(true)).build(),
        7 => AttrClass::builder("ntp.mode.reservedForPrivate").typ("@novalue").cast(cast::Const(true)).build(),
    };
}

genet_dissectors!(NtpDissector {});
