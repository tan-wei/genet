use slice::Slice;
use std::{
    convert::Into,
    io::{Error, ErrorKind, Result},
};

#[derive(Debug, Clone, PartialEq)]
pub enum Variant {
    Nil,
    Bool(bool),
    Int64(i64),
    UInt64(u64),
    Float64(f64),
    String(Box<str>),
    Buffer(Box<[u8]>),
    Slice(Slice),
}

impl Variant {
    pub fn get_u64(&self) -> Result<u64> {
        match self {
            Variant::UInt64(v) => Ok(*v),
            _ => Err(Error::new(ErrorKind::InvalidData, "Wrong type")),
        }
    }
}

impl Into<Variant> for bool {
    fn into(self) -> Variant {
        Variant::Bool(self)
    }
}

impl Into<Variant> for i8 {
    fn into(self) -> Variant {
        Variant::Int64(self as i64)
    }
}

impl Into<Variant> for i16 {
    fn into(self) -> Variant {
        Variant::Int64(self as i64)
    }
}

impl Into<Variant> for i32 {
    fn into(self) -> Variant {
        Variant::Int64(self as i64)
    }
}

impl Into<Variant> for i64 {
    fn into(self) -> Variant {
        Variant::Int64(self)
    }
}

impl Into<Variant> for u8 {
    fn into(self) -> Variant {
        Variant::UInt64(self as u64)
    }
}

impl Into<Variant> for u16 {
    fn into(self) -> Variant {
        Variant::UInt64(self as u64)
    }
}

impl Into<Variant> for u32 {
    fn into(self) -> Variant {
        Variant::UInt64(self as u64)
    }
}

impl Into<Variant> for u64 {
    fn into(self) -> Variant {
        Variant::UInt64(self)
    }
}

impl Into<Variant> for f32 {
    fn into(self) -> Variant {
        Variant::Float64(self as f64)
    }
}

impl Into<Variant> for f64 {
    fn into(self) -> Variant {
        Variant::Float64(self)
    }
}

impl Into<Variant> for Box<str> {
    fn into(self) -> Variant {
        Variant::String(self)
    }
}

impl Into<Variant> for Box<[u8]> {
    fn into(self) -> Variant {
        Variant::Buffer(self)
    }
}

impl Into<Variant> for Slice {
    fn into(self) -> Variant {
        Variant::Slice(self)
    }
}
