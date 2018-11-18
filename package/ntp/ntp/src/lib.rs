extern crate genet_sdk;

use genet_sdk::{cast, decoder::*, prelude::*};

struct NtpWorker {}

impl Worker for NtpWorker {
    fn decode(
        &mut self,
        _ctx: &mut Context,
        stack: &LayerStack,
        parent: &mut Parent,
    ) -> Result<Status> {
        if parent.id() != token!("udp") {
            return Ok(Status::Skip);
        }

        let data;

        if let Some(payload) = parent.payloads().next() {
            data = payload.data();
        } else {
            return Ok(Status::Skip);
        }

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

        let mut layer = Layer::new(&NTP_CLASS, data);
        let leap_type = LEAP_ATTR_HEADER.try_get(&layer)?.try_into()?;

        let leap = get_leap(leap_type);
        if let Some(attr) = leap {
            layer.add_attr(attr, 0..1);
        }

        let mode_type = MODE_ATTR_HEADER.try_get(&layer)?.try_into()?;

        let mode = get_mode(mode_type);
        if let Some(attr) = mode {
            layer.add_attr(attr, 0..1);
        }

        let stratum: u8 = STRATUM_ATTR_HEADER.try_get(&layer)?.try_into()?;
        if stratum >= 2 {
            layer.add_attr(&ID_IP_ATTR, 12..16);
        } else {
            layer.add_attr(&ID_ATTR, 12..16);
        }

        parent.add_child(layer);
        Ok(Status::Done)
    }
}

#[derive(Clone)]
struct NtpDecoder {}

impl Decoder for NtpDecoder {
    fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
        Box::new(NtpWorker {})
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            exec_type: ExecType::ParallelSync,
            ..Metadata::default()
        }
    }
}

def_layer_class!(
    NTP_CLASS,
    "ntp",
    header: &LEAP_ATTR,
    header: &VERSION_ATTR,
    header: &MODE_ATTR,
    header: &STRATUM_ATTR,
    header: &POLL_ATTR,
    header: &PRECISION_ATTR,
    header: &RDELAY_ATTR,
    header: &RDELAY_SEC_ATTR,
    header: &RDELAY_FRA_ATTR,
    header: &RDISP_ATTR,
    header: &RDISP_SEC_ATTR,
    header: &RDISP_FRA_ATTR,
    header: &REFTS_ATTR,
    header: &REFTS_SEC_ATTR,
    header: &REFTS_FRA_ATTR,
    header: &ORITS_ATTR,
    header: &ORITS_SEC_ATTR,
    header: &ORITS_FRA_ATTR,
    header: &RECTS_ATTR,
    header: &RECTS_SEC_ATTR,
    header: &RECTS_FRA_ATTR,
    header: &TRATS_ATTR,
    header: &TRATS_SEC_ATTR,
    header: &TRATS_FRA_ATTR
);

def_attr!(LEAP_ATTR_HEADER,  &LEAP_ATTR, range: 0..1);
def_attr!(MODE_ATTR_HEADER,  &MODE_ATTR, range: 0..1);
def_attr!(STRATUM_ATTR_HEADER,  &STRATUM_ATTR, range: 1..2);

def_attr_class!(LEAP_ATTR, "ntp.leapIndicator",
    typ: "@enum",
    cast: cast::UInt8().map(|v| v >> 6),
    range: 0..1
);

def_attr_class!(VERSION_ATTR, "ntp.version",
    cast: cast::UInt8().map(|v| (v >> 3) & 0b111),
    range: 0..1
);

def_attr_class!(MODE_ATTR, "ntp.mode",
    typ: "@enum",
    cast: cast::UInt8().map(|v| v & 0b111),
    range: 0..1
);

def_attr_class!(STRATUM_ATTR, "ntp.stratum",
    cast: cast::UInt8(),
    range: 1..2
);

def_attr_class!(POLL_ATTR, "ntp.pollInterval",
    cast: cast::UInt8(),
    range: 2..3
);

def_attr_class!(PRECISION_ATTR, "ntp.precision",
    cast: cast::UInt8(),
    range: 3..4
);

def_attr_class!(RDELAY_ATTR, "ntp.rootDelay",
    cast: cast::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)),
    range: 4..8
);

def_attr_class!(
    RDELAY_SEC_ATTR,
    "ntp.rootDelay.seconds",
    cast: cast::UInt16BE(),
    range: 4..6
);

def_attr_class!(
    RDELAY_FRA_ATTR,
    "ntp.rootDelay.fraction",
    cast: cast::UInt16BE(),
    range: 6..8
);

def_attr_class!(RDISP_ATTR, "ntp.rootDispersion",
    cast: cast::UInt32BE().map(|v| (v >> 16) as f64 + ((v & 0xffff) as f64 / 65536f64)),
    range: 8..12
);

def_attr_class!(
    RDISP_SEC_ATTR,
    "ntp.rootDispersion.seconds",
    cast: cast::UInt16BE(),
    range: 8..10
);

def_attr_class!(
    RDISP_FRA_ATTR,
    "ntp.rootDispersion.fraction",
    cast: cast::UInt16BE(),
    range: 10..12
);

def_attr_class!(ID_ATTR, "ntp.identifier", cast: cast::ByteSlice());

def_attr_class!(ID_IP_ATTR, "ntp.identifier",
    typ: "@ipv4:addr",
    cast: cast::ByteSlice()
);

def_attr_class!(REFTS_ATTR, "ntp.referenceTs",
    typ: "@ntp:time",
    cast: cast::UInt64BE().map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64)),
    range: 16..24
);

def_attr_class!(
    REFTS_SEC_ATTR,
    "ntp.referenceTs.seconds",
    cast: cast::UInt32BE(),
    range: 16..20
);

def_attr_class!(
    REFTS_FRA_ATTR,
    "ntp.referenceTs.fraction",
    cast: cast::UInt32BE(),
    range: 20..24
);

def_attr_class!(ORITS_ATTR, "ntp.originateTs",
    typ: "@ntp:time",
    cast: cast::UInt64BE().map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64)),
    range: 24..32
);

def_attr_class!(
    ORITS_SEC_ATTR,
    "ntp.originateTs.seconds",
    cast: cast::UInt32BE(),
    range: 24..28
);

def_attr_class!(
    ORITS_FRA_ATTR,
    "ntp.originateTs.fraction",
    cast: cast::UInt32BE(),
    range: 28..32
);

def_attr_class!(RECTS_ATTR, "ntp.receiveTs",
    typ: "@ntp:time",
    cast: cast::UInt64BE().map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64)),
    range: 32..40
);

def_attr_class!(
    RECTS_SEC_ATTR,
    "ntp.receiveTs.seconds",
    cast: cast::UInt32BE(),
    range: 32..36
);

def_attr_class!(
    RECTS_FRA_ATTR,
    "ntp.receiveTs.fraction",
    cast: cast::UInt32BE(),
    range: 36..40
);

def_attr_class!(TRATS_ATTR, "ntp.transmitTs",
    typ: "@ntp:time",
    cast: cast::UInt64BE().map(|v| (v >> 32) as f64 + ((v & 0xffff_ffff) as f64 / 4294967296f64)),
    range: 40..48
);

def_attr_class!(
    TRATS_SEC_ATTR,
    "ntp.transmitTs.seconds",
    cast: cast::UInt32BE(),
    range: 40..44
);

def_attr_class!(
    TRATS_FRA_ATTR,
    "ntp.transmitTs.fraction",
    cast: cast::UInt32BE(),
    range: 44..48
);

fn get_leap(val: u64) -> Option<&'static AttrClass> {
    match val {
        0 => Some(attr_class_lazy!("ntp.leapIndicator.noWarning", typ: "@novalue", value: true)),
        1 => Some(attr_class_lazy!("ntp.leapIndicator.sec61", typ: "@novalue", value: true)),
        2 => Some(attr_class_lazy!("ntp.leapIndicator.sec59", typ: "@novalue", value: true)),
        3 => Some(attr_class_lazy!("ntp.leapIndicator.unknown", typ: "@novalue", value: true)),
        _ => None,
    }
}

fn get_mode(val: u64) -> Option<&'static AttrClass> {
    match val {
        0 => Some(attr_class_lazy!("ntp.mode.reserved", typ: "@novalue", value: true)),
        1 => Some(attr_class_lazy!("ntp.mode.symmetricActive", typ: "@novalue", value: true)),
        2 => Some(attr_class_lazy!("ntp.mode.symmetricPassive", typ: "@novalue", value: true)),
        3 => Some(attr_class_lazy!("ntp.mode.client", typ: "@novalue", value: true)),
        4 => Some(attr_class_lazy!("ntp.mode.server", typ: "@novalue", value: true)),
        5 => Some(attr_class_lazy!("ntp.mode.broadcast", typ: "@novalue", value: true)),
        6 => Some(attr_class_lazy!("ntp.mode.controlMessage", typ: "@novalue", value: true)),
        7 => Some(attr_class_lazy!("ntp.mode.reservedForPrivate", typ: "@novalue", value: true)),
        _ => None,
    }
}

genet_decoders!(NtpDecoder {});
