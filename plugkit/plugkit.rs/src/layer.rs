//! Layer values.
//!
//! Type Layer represents a layer of a protocol stack.

use super::token::Token;
use super::attr::Attr;
use super::payload::Payload;
use super::context::Context;
use super::symbol;
use super::range::Range;
use std::slice;

extern crate libc;

/// Maximum value of worker ID.
pub const MAX_WORKER: u8 = 16;

/// A layer object.
#[derive(Debug)]
#[repr(C)]
pub struct Layer {
    id: Token,
    data: u32,
    parent: *mut Layer,
    range: (u32, u32),
}

/// Confidence level.
#[derive(Debug)]
pub enum Confidence {
    Error = 0,
    Possible = 1,
    Probable = 2,
    Exact = 3,
}

impl Layer {
    /// Returns the ID of `self`.
    pub fn id(&self) -> Token {
        self.id
    }

    /// Finds the first attribute with `id`.
    pub fn attr(&self, id: Token) -> Option<&Attr> {
        unsafe { symbol::Layer_attr.unwrap()(self, id).as_ref() }
    }

    /// Returns an `Iterator` for the payloads of `self`.
    pub fn payloads(&self) -> Box<Iterator<Item = &Payload>> {
        unsafe {
            let mut size: libc::size_t = 0;
            let ptr = symbol::Layer_payloads.unwrap()(self, &mut size);
            let s = slice::from_raw_parts(ptr, size);
            Box::new(s.iter().map(|elem| &**elem))
        }
    }

    /// Allocates a new `Layer` and add it as a child.
    pub fn add_layer(&mut self, ctx: &mut Context, id: Token) -> &mut Layer {
        unsafe { &mut *symbol::Layer_addLayer.unwrap()(self, ctx, id) }
    }

    /// Allocates a new `Attr` and add it as a child.
    pub fn add_attr(&mut self, ctx: &mut Context, id: Token) -> &mut Attr {
        unsafe { &mut *symbol::Layer_addAttr.unwrap()(self, ctx, id) }
    }

    /// Allocates a new alias `Attr`.
    pub fn add_attr_alias(&mut self, ctx: &mut Context, alias: Token, target: Token) {
        unsafe { symbol::Layer_addAttrAlias.unwrap()(self, ctx, alias, target) }
    }

    /// Allocates a new `Payload` and add it as a child.
    pub fn add_payload(&mut self, ctx: &mut Context) -> &mut Payload {
        unsafe { &mut *symbol::Layer_addPayload.unwrap()(self, ctx) }
    }

    /// Allocates a new error `Attr`.
    pub fn add_error(&mut self, ctx: &mut Context, id: Token, msg: &str) {
        unsafe {
            symbol::Layer_addError.unwrap()(self, ctx, id, msg.as_ptr() as *const i8, msg.len())
        }
    }

    /// Adds `id` as a tag of `self`.
    pub fn add_tag(&mut self, ctx: &mut Context, id: Token) {
        unsafe {
            symbol::Layer_addTag.unwrap()(self, ctx, id);
        }
    }

    /// Returns a reference to the parent of `self`.
    pub fn parent(&self) -> Option<&Layer> {
        if self.is_root() || self.parent.is_null() {
            None
        } else {
            unsafe { Some(&*self.parent) }
        }
    }

    /// Returns a mutable reference to the parent of `self`.
    pub fn parent_mut(&mut self) -> Option<&mut Layer> {
        if self.is_root() || self.parent.is_null() {
            None
        } else {
            unsafe { Some(&mut *self.parent) }
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

    /// Returns the worker ID of `self`.
    pub fn worker(&self) -> u8 {
        (self.data & 0b1111) as u8
    }

    /// Sets the worker ID of `self`.
    pub fn set_worker(&mut self, worker: u8) {
        self.data = (self.data & !0b1111) | (worker % MAX_WORKER) as u32
    }

    fn is_root(&self) -> bool {
        ((self.data >> 6) & 0b1) != 0
    }
}
