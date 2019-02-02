//! Token values.
//!
//! Token is a process-wide constant integer value associated with a string,
//! similar to Symbol in Ruby or Atom in Erlang.

/// Returns a token corresponded with the given string.
///
/// # Examples
/// ```
/// # #[macro_use] extern crate genet_sdk;
/// # fn main() {
/// let tk = token!("tcp");
/// # }
/// ```
#[macro_export]
macro_rules! token {
    () => {{
        use $crate::token::Token;
        Token::null()
    }};
    ($name:expr) => {{
        use $crate::token::Token;
        thread_local!(
            static TOKEN: Token = { Token::from($name) };
        );
        TOKEN.with(|&t| t)
    }};
}

pub use genet_abi::token::Token;
