use super::token::Token;
use super::attr::Attr;
use super::payload::Payload;
use super::context::Context;
use super::symbol;
use super::range::Range;
use std::slice;

extern crate libc;

pub enum Layer {}

#[derive(Debug)]
pub enum Confidence {
    Error = 0,
    Possible = 1,
    Probable = 2,
    Exact = 3,
}

impl Layer {
    pub fn id(&self) -> Token {
        unsafe { symbol::Layer_id.unwrap()(self) }
    }

    pub fn attr(&self, id: Token) -> Option<&Attr> {
        unsafe { symbol::Layer_attr.unwrap()(self, id).as_ref() }
    }

    pub fn payloads(&self) -> Box<Iterator<Item = &Payload>> {
        unsafe {
            let mut size: libc::size_t = 0;
            let ptr = symbol::Layer_payloads.unwrap()(self, &mut size);
            let s = slice::from_raw_parts(ptr, size);
            Box::new(s.iter().map(|elem| &**elem))
        }
    }

    pub fn add_layer(&mut self, ctx: &mut Context, id: Token) -> &mut Layer {
        unsafe { &mut *symbol::Layer_addLayer.unwrap()(self, ctx, id) }
    }

    pub fn add_sublayer(&mut self, ctx: &mut Context, id: Token) -> &mut Layer {
        unsafe { &mut *symbol::Layer_addSubLayer.unwrap()(self, ctx, id) }
    }

    pub fn add_attr(&mut self, ctx: &mut Context, id: Token) -> &mut Attr {
        unsafe { &mut *symbol::Layer_addAttr.unwrap()(self, ctx, id) }
    }

    pub fn add_payload(&mut self, ctx: &mut Context) -> &mut Payload {
        unsafe { &mut *symbol::Layer_addPayload.unwrap()(self, ctx) }
    }

    pub fn add_tag(&mut self, id: Token) {
        unsafe {
            symbol::Layer_addTag.unwrap()(self, id);
        }
    }

    pub fn range(&self) -> Range {
        unsafe {
            let (start, end) = symbol::Layer_range.unwrap()(self);
            Range {
                start: start,
                end: end,
            }
        }
    }

    pub fn set_range(&mut self, range: Range) {
        unsafe { symbol::Layer_setRange.unwrap()(self, (range.start, range.end)) }
    }

    pub fn confidence(&self) -> Confidence {
        unsafe { symbol::Layer_confidence.unwrap()(self) }
    }

    pub fn set_confidence(&mut self, conf: Confidence) {
        unsafe { symbol::Layer_setConfidence.unwrap()(self, conf) }
    }

    pub fn worker(&self) -> u8 {
        unsafe { symbol::Layer_worker.unwrap()(self) }
    }

    pub fn set_worker(&mut self, worker: u8) {
        unsafe { symbol::Layer_setWorker.unwrap()(self, worker) }
    }
}
