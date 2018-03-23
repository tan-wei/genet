//! Attribute values.
//!
//! Type Attr represents an attribute of a Layer.

extern crate libc;

use super::token::Token;
use super::range::Range;
use super::variant::{Value, Variant};

/// An attribute object.
#[repr(C)]
pub struct Attr {
    id: Token,
    typ: Token,
    val: Variant,
    range: (u32, u32),
}

impl<T> Value<T> for Attr
where
    Variant: Value<T>,
{
    fn get(&self) -> T {
        Value::get(self.value())
    }

    fn set(&mut self, val: &T) {
        Value::set(self.value_mut(), val)
    }
}

pub trait ResultValue<T> {
    fn set_with_range(&mut self, val: &(T, Range));
}

impl<T> ResultValue<T> for Attr
where
    Variant: Value<T>,
{
    fn set_with_range(&mut self, val: &(T, Range)) {
        Value::set(self.value_mut(), &val.0);
        self.set_range(&val.1)
    }
}

impl Attr {
    /// Returns the ID of `self`.
    pub fn id(&self) -> Token {
        self.id
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

    /// Returns a reference to the value of `self`.
    pub fn value(&self) -> &Variant {
        &self.val
    }

    /// Returns a mutable reference to the value of `self`.
    pub fn value_mut(&mut self) -> &mut Variant {
        &mut self.val
    }
}
