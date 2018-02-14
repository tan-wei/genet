//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::variant::Variant;
use super::symbol;
use std::ffi::CString;

pub enum Context {}

impl Context {
    pub fn get_option(&self, name: &str) -> &Variant {
        unsafe {
            let cstr = CString::new(name).unwrap_or_else(|_| CString::new("").unwrap());
            &*symbol::Context_getOption.unwrap()(self, cstr.as_ptr())
        }
    }
}
