//! Token values.
//!
//! Token is a process-wide constant integer value associated with a string,
//! similar to Symbol in Ruby or Atom in Erlang.

use super::symbol;
use std::str;
use std::ffi::CStr;

pub type Token = u32;

pub fn get(name: &str) -> Token {
    unsafe { symbol::Token_literal_.unwrap()(name.as_ptr() as *const i8, name.len()) }
}

pub fn string(token: Token) -> &'static str {
    unsafe {
        let slice = CStr::from_ptr(symbol::Token_string.unwrap()(token));
        str::from_utf8_unchecked(slice.to_bytes())
    }
}

pub fn join(prefix: Token, token: Token) -> Token {
    unsafe { symbol::Token_join.unwrap()(prefix, token) }
}

/// Return a token corresponded with the given string.
///
/// # Examples
///
/// ```
/// #[macro_use]
/// use plugkit::token;
///
/// let tk = token!("tcp");
/// ```
#[macro_export]
macro_rules! token {
    ($name:expr) => {{
        thread_local! (
            static TOKEN: plugkit::token::Token = plugkit::token::get($name as &'static str);
        );
        TOKEN.with(|&t| t)
    }}
}
