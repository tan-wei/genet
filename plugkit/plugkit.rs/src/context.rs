//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::variant::Variant;
use super::layer::Layer;
use super::symbol;
use super::token::Token;
use std::ffi::CString;

#[repr(C)]
pub struct Context {
    close_stream: bool
}

impl Context {
    pub fn get_option(&self, name: &str) -> &Variant {
        unsafe {
            let cstr = CString::new(name).unwrap_or_else(|_| CString::new("").unwrap());
            &*symbol::Context_getOption.unwrap()(self, cstr.as_ptr())
        }
    }

    pub fn close_stream(&mut self) {
        self.close_stream = true
    }

    pub fn add_layer_linkage(&mut self, token: Token, id: u64, layer: &mut Layer) {
        unsafe {
            symbol::Context_addLayerLinkage.unwrap()(self, token, id, layer);
        }
    }
}
