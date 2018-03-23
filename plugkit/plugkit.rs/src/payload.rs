//! Payload values.
//!
//! Type Attr represents an attribute of a Payload.

use super::token::Token;
use super::range::Range;
use super::symbol;
use std::slice;

extern crate libc;

/// A payload object.
#[repr(C)]
pub struct Payload {
    typ: Token,
    len: u32,
    range: (u32, u32),
}

impl Payload {
    /// Adds `data` as a slice.
    pub fn add_slice(&mut self, data: &'static [u8]) {
        unsafe {
            symbol::Payload_addSlice.unwrap()(self, (data.as_ptr(), data.len()));
        }
    }

    /// Returns an `Iterator` for the slices.
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

    /// Returns the range of `self`.
    pub fn range(&self) -> Range {
        Range {
            start: self.range.0,
            end: self.range.1,
        }
    }

    /// Sets the range of `self`.
    pub fn set_range(&mut self, range: &Range) {
        self.range = (range.start, range.end)
    }

    /// Returns the type of `self`.
    pub fn typ(&self) -> Token {
        self.typ
    }

    /// Sets the type of `self`.
    pub fn set_typ(&mut self, id: Token) {
        self.typ = id
    }
}
