#[repr(C)]
#[derive(Clone, Copy, PartialEq, Debug)]
pub struct Token(u64);

impl Token {
    pub fn new(id: u64) -> Token {
        Token(id)
    }

    pub fn null() -> Token {
        Self::new(0)
    }

    pub fn as_u64(self) -> u64 {
        self.0
    }
}

impl Into<u64> for Token {
    fn into(self) -> u64 {
        self.0
    }
}

impl From<u64> for Token {
    fn from(id: u64) -> Token {
        Self::new(id)
    }
}
