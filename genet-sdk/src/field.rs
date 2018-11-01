//! Attribute field types.

use cast;
pub use genet_abi::field::{Detach, Node};
use genet_abi::{
    attr::{AttrContext, AttrField, AttrList, SizedAttrField},
    variant::Variant,
};

pub struct Uint8 {}

impl Uint8 {
    fn map<T: Into<Variant>, F: Fn(u8) -> T>(self, f: F) -> Self {
        self
    }
}

impl Default for Uint8 {
    fn default() -> Self {
        Self {}
    }
}

impl AttrField for Uint8 {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        AttrList::from_ctx(ctx, cast::UInt8())
    }
}

impl SizedAttrField for Uint8 {
    fn bit_size(&self) -> usize {
        8
    }
}

macro_rules! impl_pad {
    ($id:ident, $size:expr) => {
        #[derive(Default)]
        pub struct $id {}

        impl AttrField for $id {
            fn init(&mut self, ctx: &AttrContext) -> AttrList {
                AttrList::from_ctx(ctx, cast::UInt8())
            }
        }

        impl SizedAttrField for $id {
            fn bit_size(&self) -> usize {
                $size
            }
        }
    };
}

impl_pad!(BytePad1, 8 * 1);
impl_pad!(BytePad2, 8 * 2);
impl_pad!(BytePad3, 8 * 3);
impl_pad!(BytePad4, 8 * 4);
impl_pad!(BytePad5, 8 * 5);
impl_pad!(BytePad6, 8 * 6);
impl_pad!(BytePad7, 8 * 7);
impl_pad!(BytePad8, 8 * 8);

pub struct BytePad {
    len: usize,
}

impl BytePad {
    pub fn new(len: usize) -> Self {
        Self { len }
    }
}

impl AttrField for BytePad {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        AttrList::from_ctx(ctx, cast::UInt8())
    }
}

impl SizedAttrField for BytePad {
    fn bit_size(&self) -> usize {
        self.len * 8
    }
}
