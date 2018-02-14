//! Error values.
//!
//! Type Error represents an error associated with a Layer.

use super::token::Token;

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
}
