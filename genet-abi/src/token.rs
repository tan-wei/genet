use fnv::FnvHashMap;
use lazy_static::lazy_static;
use parking_lot::Mutex;
use std::{cell::RefCell, collections::hash_map::Entry, fmt, slice, str};

/// A token value.
#[repr(transparent)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Token(u128);

impl Token {
    /// Returns a null token.
    pub fn null() -> Token {
        Token(0)
    }
}

impl Into<u128> for Token {
    fn into(self) -> u128 {
        self.0
    }
}

impl<'a> From<&'a Token> for Token {
    fn from(id: &'a Token) -> Token {
        *id
    }
}

impl From<u128> for Token {
    fn from(id: u128) -> Token {
        Token(id)
    }
}

impl From<String> for Token {
    fn from(id: String) -> Token {
        Token::from(id.as_str())
    }
}

impl From<&String> for Token {
    fn from(id: &String) -> Token {
        Token::from(id.as_str())
    }
}

impl<'a> From<&'a str> for Token {
    fn from(id: &'a str) -> Token {
        token(id)
    }
}

fn token(id: &str) -> Token {
    if id.is_empty() {
        Token::null()
    } else {
        let mut hash = 0x6c62272e07bb014262b821756295c58d;
        for byte in id.as_bytes() {
            hash = hash ^ (*byte as u128);
            hash = hash.wrapping_mul(0x1000000000000000000013b);
        }
        hash.into()
    }
}

#[cfg(test)]
mod tests {
    use crate::token::Token;

    #[test]
    fn token() {
        assert_eq!(Token::from(""), Token::null());
        /*
        let token = Token::from("eth");
        assert_eq!(token.to_string(), "eth");
        let token = Token::from("[eth]");
        assert_eq!(token.to_string(), "[eth]");
        let token = Token::from("eth");
        assert_eq!(token.to_string(), "eth");
        let token = Token::from("");
        assert_eq!(token.to_string(), "");
        let token = Token::from("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(token.to_string(), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(Token::from(1000).to_string(), "");
        */
    }
}
