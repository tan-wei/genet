//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::variant::Variant;
use super::symbol;
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
}
