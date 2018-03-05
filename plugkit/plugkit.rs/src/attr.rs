//! Attribute values.
//!
//! Type Attr represents an attribute of a Layer.

extern crate libc;

use super::token::Token;
use super::range::Range;
use super::variant::{Value, ValueString, Variant};

#[repr(C)]
pub struct Attr {
    id: Token,
    typ: Token,
    val: Variant,
    range: (u32, u32)
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

impl ValueString for Attr
{
    fn get(&self) -> String {
        ValueString::get(self.value())
    }

    fn set_copy(&mut self, val: &str) {
        ValueString::set_copy(self.value_mut(), val)
    }

    fn set_ref(&mut self, val: &'static str) {
        ValueString::set_ref(self.value_mut(), val)
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
    pub fn id(&self) -> Token {
        self.id
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

    pub fn value(&self) -> &Variant {
        &self.val
    }

    pub fn value_mut(&mut self) -> &mut Variant {
        &mut self.val
    }

    pub fn set_nil(&mut self) {
        self.value_mut().set_nil()
    }
}
