use super::token::Token;
use super::attr::Attr;
use super::payload::Payload;
use super::context::Context;
use super::symbol;
use super::range::Range;
use std::mem;
use std::slice;

extern crate libc;

enum Frame {}

pub const MAX_WORKER: u8 = 16;

#[repr(C)]
pub struct Layer {
    id: Token,
    data: u8,
    parent: *mut Layer,
    master: *const Layer,
    frame: *const Frame,
    range: (libc::size_t, libc::size_t)
}

#[derive(Debug)]
pub enum Confidence {
    Error = 0,
    Possible = 1,
    Probable = 2,
    Exact = 3,
}

impl Layer {
    pub fn id(&self) -> Token {
        self.id
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
        Range {
            start: self.range.0,
            end: self.range.1,
        }
    }

    pub fn set_range(&mut self, range: &Range) {
        self.range = (range.start, range.end)
    }

    pub fn confidence(&self) -> Confidence {
        unsafe { mem::transmute((self.data >> 4) & 0b0000_0011) }
    }

    pub fn set_confidence(&mut self, conf: Confidence) {
        self.data = (self.data & 0b1100_1111) | ((conf as u8) << 4)
    }

    pub fn worker(&self) -> u8 {
        self.data & 0b0000_1111
    }

    pub fn set_worker(&mut self, worker: u8) {
        self.data = (self.data & 0b1111_0000) | (worker % MAX_WORKER)
    }
}
