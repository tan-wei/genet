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

pub trait Value<T> {
    fn try_into(self) -> Result<T>;
}

impl Value<String> for Variant {
    fn try_into(self) -> Result<String> {
        match self {
            Variant::String(val) => Ok(val.to_string()),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<Vec<u8>> for Variant {
    fn try_into(self) -> Result<Vec<u8>> {
        match self {
            Variant::String(val) => Ok(val.to_string().into_bytes()),
            Variant::Buffer(val) => Ok(val.into_vec()),
            Variant::Slice(val) => Ok(val.as_ref().to_vec()),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<Slice> for Variant {
    fn try_into(self) -> Result<Slice> {
        match self {
            Variant::Slice(val) => Ok(val),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<u64> for Variant {
    fn try_into(self) -> Result<u64> {
        match self {
            Variant::Nil => Ok(0),
            Variant::Bool(val) => Ok(if val { 1 } else { 0 }),
            Variant::Int64(val) => Ok(val as u64),
            Variant::UInt64(val) => Ok(val as u64),
            Variant::Float64(val) => Ok(val as u64),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<usize> for Variant {
    fn try_into(self) -> Result<usize> {
        Value::<u64>::try_into(self).map(|v| v as usize)
    }
}

impl Value<u32> for Variant {
    fn try_into(self) -> Result<u32> {
        Value::<u64>::try_into(self).map(|v| v as u32)
    }
}

impl Value<u16> for Variant {
    fn try_into(self) -> Result<u16> {
        Value::<u64>::try_into(self).map(|v| v as u16)
    }
}

impl Value<u8> for Variant {
    fn try_into(self) -> Result<u8> {
        Value::<u64>::try_into(self).map(|v| v as u8)
    }
}

impl Value<i64> for Variant {
    fn try_into(self) -> Result<i64> {
        match self {
            Variant::Nil => Ok(0),
            Variant::Bool(val) => Ok(if val { 1 } else { 0 }),
            Variant::Int64(val) => Ok(val as i64),
            Variant::UInt64(val) => Ok(val as i64),
            Variant::Float64(val) => Ok(val as i64),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<isize> for Variant {
    fn try_into(self) -> Result<isize> {
        Value::<i64>::try_into(self).map(|v| v as isize)
    }
}

impl Value<i32> for Variant {
    fn try_into(self) -> Result<i32> {
        Value::<i64>::try_into(self).map(|v| v as i32)
    }
}

impl Value<i16> for Variant {
    fn try_into(self) -> Result<i16> {
        Value::<i64>::try_into(self).map(|v| v as i16)
    }
}

impl Value<i8> for Variant {
    fn try_into(self) -> Result<i8> {
        Value::<i64>::try_into(self).map(|v| v as i8)
    }
}

impl Value<f64> for Variant {
    fn try_into(self) -> Result<f64> {
        match self {
            Variant::Nil => Ok(0f64),
            Variant::Bool(val) => Ok(if val { 1f64 } else { 0f64 }),
            Variant::Int64(val) => Ok(val as f64),
            Variant::UInt64(val) => Ok(val as f64),
            Variant::Float64(val) => Ok(val as f64),
            _ => Err(Error::new(ErrorKind::InvalidData, "wrong type")),
        }
    }
}

impl Value<f32> for Variant {
    fn try_into(self) -> Result<f32> {
        Value::<f64>::try_into(self).map(|v| v as f32)
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
