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

#[cfg(test)]
mod tests {
    use crate::token::Token;

    #[test]
    fn token() {
        assert_eq!(token!(), Token::null());
        let token = token!("eth");
        assert_eq!(token.as_str(), "eth");
        let token = token!("[eth]");
        assert_eq!(token.as_str(), "[eth]");
        let token = token!("eth");
        assert_eq!(token.as_str(), "eth");
        let token = token!();
        assert_eq!(token.as_str(), "");
        let token = token!("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(token.as_str(), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
    }
}
