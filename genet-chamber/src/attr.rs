#![forbid(unsafe_code)]

use crate::token::Token;
use std::{borrow::Cow, ops::Range};

pub struct AttrType<'a> {
    id: Token,
    path: Cow<'a, str>,
    bit_range: Range<usize>,
    ty: Cow<'a, str>,
    name: Cow<'a, str>,
    description: Cow<'a, str>,
    aliases: Cow<'a, [Token]>,
}

impl<'a> AttrType<'a> {
    pub fn id(&self) -> Token {
        self.id
    }

    pub fn path(&self) -> &str {
        &self.path
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

    pub fn aliases(&self) -> &[Token] {
        &self.aliases
    }
}
