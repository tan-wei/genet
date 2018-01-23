use super::token::Token;
use super::range::Range;
use super::symbol;
use std::slice;

extern crate libc;

pub enum Payload {}

impl Payload {
    pub fn add_slice(&mut self, data: &'static[u8]) {
        unsafe {
            let begin = data.as_ptr();
            let end = begin.offset(data.len() as isize);
            symbol::Payload_addSlice.unwrap()(self, (begin, end));
        }
    }

    pub fn slices(&self) -> Box<Iterator<Item=&'static[u8]>> {
        unsafe {
            let mut size: libc::size_t = 0;
            let ptr = symbol::Payload_slices.unwrap()(self, &mut size);
            let s = slice::from_raw_parts(ptr, size);
            Box::new(s.iter().map(|elem| {
                let (begin, end) = *elem;
                slice::from_raw_parts(begin, (end as usize) - (begin as usize))
            }))
        }
    }

    pub fn range(&self) -> Range {
        unsafe {
            let (start, end) = symbol::Payload_range.unwrap()(self);
            Range { start: start, end: end }
        }
    }

    pub fn set_range(&mut self, range: &Range) {
        unsafe {
            symbol::Payload_setRange.unwrap()(self, (range.start, range.end))
        }
    }

    pub fn typ(&self) -> Token {
        unsafe {
            symbol::Payload_type.unwrap()(self)
        }
    }

    pub fn set_typ(&mut self, id: Token) {
        unsafe {
            symbol::Payload_setType.unwrap()(self, id)
        }
    }
}
