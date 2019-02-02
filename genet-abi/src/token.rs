use std::str;

/// A token value.
#[repr(transparent)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Token(u128);

impl Token {
    /// Returns a null token.
    #[inline]
    pub fn null() -> Token {
        Token(0)
    }
}

impl Into<u128> for Token {
    #[inline]
    fn into(self) -> u128 {
        self.0
    }
}

impl<'a> From<&'a Token> for Token {
    #[inline]
    fn from(id: &'a Token) -> Token {
        *id
    }
}

impl From<u128> for Token {
    #[inline]
    fn from(id: u128) -> Token {
        Token(id)
    }
}

impl From<String> for Token {
    #[inline]
    fn from(id: String) -> Token {
        Token::from(id.as_str())
    }
}

impl From<&String> for Token {
    #[inline]
    fn from(id: &String) -> Token {
        Token::from(id.as_str())
    }
}

impl<'a> From<&'a str> for Token {
    #[inline]
    fn from(id: &'a str) -> Token {
        token(id)
    }
}

#[inline]
fn token(id: &str) -> Token {
    id.as_bytes()
        .iter()
        .fold(0x6c62_272e_07bb_0142_62b8_2175_6295_c58d, |acc, x| {
            (acc ^ u128::from(*x)).wrapping_mul(0x0100_0000_0000_0000_0000_013b)
        })
        .into()
}
