//! Error values.
//!
//! Type Error represents an error associated with a Layer.

use super::token::Token;
use super::symbol;
use std::ffi::CStr;

#[repr(C)]
pub struct Error {
  id: Token,
  target: Token
}

impl Error {
    pub fn id(&self) -> Token {
      self.id
    }

    pub fn target(&self) -> Token {
      self.target
    }

    pub fn set_target(&mut self, target: Token) {
      self.target = target
    }

    pub fn message(&self) -> String {
        unsafe {
            let slice = CStr::from_ptr(symbol::Error_message.unwrap()(self));
            String::from_utf8_unchecked(slice.to_bytes().to_vec())
        }
    }

    pub fn set_message(&mut self, val: &str) {
        unsafe { symbol::Error_setMessage.unwrap()(self, val.as_ptr() as *const i8) }
    }
}
