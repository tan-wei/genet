use env;
use std::fmt;

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Debug)]
pub struct Token(u64);

impl Token {
    pub fn null() -> Token {
        Token(0)
    }

    pub fn as_u64(self) -> u64 {
        self.0
    }

    pub fn to_string(self) -> String {
        env::string(self)
    }
}

impl Into<u64> for Token {
    fn into(self) -> u64 {
        self.0
    }
}

impl<'a> From<&'a Token> for Token {
    fn from(id: &'a Token) -> Token {
        *id
    }
}

impl From<u64> for Token {
    fn from(id: u64) -> Token {
        Token(id)
    }
}

impl From<String> for Token {
    fn from(id: String) -> Token {
        Token::from(id.as_str())
    }
}

impl<'a> From<&'a str> for Token {
    fn from(id: &'a str) -> Token {
        env::token(id)
    }
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.pad(&Token::to_string(*self))
    }
}
