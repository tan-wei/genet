//! Token values.
//!
//! Token is a process-wide constant integer value associated with a string,
//! similar to Symbol in Ruby or Atom in Erlang.

use super::symbol;
use std::str;
use std::ffi::CStr;

pub type Token = u32;

/// Returns a token corresponded with `name`.
///
/// # Examples
///
/// ```
/// use plugkit::token;
///
/// let t = token::get("");
/// assert_eq!(t, 0);
/// ```
pub fn get(name: &str) -> Token {
    unsafe { symbol::Token_literal_.unwrap()(name.as_ptr() as *const i8, name.len()) }
}

/// Returns a string corresponded with `token`.
///
/// # Examples
///
/// ```
/// use plugkit::token;
///
/// let t = token::get("eth");
/// let s = token::string(t);
/// assert_eq!(s, "eth");
/// ```
pub fn string(token: Token) -> &'static str {
    unsafe {
        let slice = CStr::from_ptr(symbol::Token_string.unwrap()(token));
        str::from_utf8_unchecked(slice.to_bytes())
    }
}

/// Returns a token corresponded with `prefix` + `name`.
///
/// Same as `token::get(format!("{}{}", prefix, name).as_str())`.
///
/// # Examples
///
/// ```
/// use plugkit::token;
///
/// let t1 = token::get("eth.src");
/// let t2 = token::concat("eth.", "src");
/// assert_eq!(t1, t2);
/// ```
pub fn concat(prefix: &str, name: &str) -> Token {
    let mut s = String::from(prefix);
    s.push_str(name);
    get(s.as_str())
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
