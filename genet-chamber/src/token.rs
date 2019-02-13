#![forbid(unsafe_code)]

#[repr(transparent)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Token(u32);
