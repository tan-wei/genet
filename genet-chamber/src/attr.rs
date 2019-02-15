#![forbid(unsafe_code)]

use crate::token::Token;
use std::{borrow::Cow, ops::Range};

pub struct Attr<'a> {
    bit_range: Range<usize>,
    ty: &'a AttrType<'a>,
}

impl<'a> Attr<'a> {
    pub fn bit_range(&self) -> Range<usize> {
        self.bit_range.clone()
    }

    pub fn byte_range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    pub fn ty(&self) -> &AttrType<'a> {
        self.ty
    }
}

pub struct AttrType<'a> {
    id: Cow<'a, str>,
    bit_range: Range<usize>,
    ty: Cow<'a, str>,
    name: Cow<'a, str>,
    description: Cow<'a, str>,
    aliases: Cow<'a, [&'a str]>,
}

impl<'a> AttrType<'a> {
    pub fn id(&self) -> &str {
        &self.id
    }

    pub fn bit_range(&self) -> Range<usize> {
        self.bit_range.clone()
    }

    pub fn byte_range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    pub fn ty(&self) -> &str {
        &self.ty
    }

    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn description(&self) -> &str {
        &self.description
    }

    pub fn aliases(&self) -> &[&str] {
        &self.aliases
    }
}

pub(crate) struct AttrTypeData {
    pub id: Token,
    pub bit_range: Range<usize>,
    pub ty: String,
    pub name: String,
    pub description: String,
    pub aliases: Vec<Token>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub(crate) struct AttrTypeInter {
    pub id: Token,
    pub bit_range_start: u64,
    pub bit_range_end: u64,
    pub ty: *const u8,
    pub ty_len: u64,
    pub name: *const u8,
    pub name_len: u64,
    pub description: *const u8,
    pub description_len: u64,
    pub aliases: *const Token,
    pub aliases_len: u64,
}

pub(crate) struct AttrTypeSet {
    pub set: Vec<AttrTypeData>,
}
