//! Payload values.
//!
//! Type Attr represents an attribute of a Payload.

use super::token::Token;
use super::range::Range;
use super::symbol;
use std::slice;

extern crate libc;

#[repr(C)]
pub struct Payload {
    typ: Token,
    range: (libc::size_t, libc::size_t)
}

impl Payload {
    pub fn add_slice(&mut self, data: &'static [u8]) {
        unsafe {
            symbol::Payload_addSlice.unwrap()(self, (data.as_ptr(), data.len()));
        }
    }

    pub fn slices(&self) -> Box<Iterator<Item = &'static [u8]>> {
        unsafe {
            let mut size: libc::size_t = 0;
            let ptr = symbol::Payload_slices.unwrap()(self, &mut size);
            let s = slice::from_raw_parts(ptr, size);
            Box::new(s.iter().map(|elem| {
                let (data, len) = *elem;
                slice::from_raw_parts(data, len)
            }))
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

    pub fn typ(&self) -> Token {
        self.typ
    }

    pub fn set_typ(&mut self, id: Token) {
        self.typ = id
    }
}
