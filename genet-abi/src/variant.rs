use crate::{result::Result, slice::ByteSlice};
use failure::err_msg;
use num_bigint::BigInt;
use num_traits::cast::ToPrimitive;
use std::convert::Into;

#[derive(Debug, Clone, PartialEq)]
pub enum Variant {
    Nil,
    Bool(bool),
    Int64(i64),
    UInt64(u64),
    Float64(f64),
    BigInt(BigInt),
    Buffer(Box<[u8]>),
    Slice(ByteSlice),
}

/// Variant value trait.
pub trait TryInto<T> {
    fn try_into(self) -> Result<T>;
}

impl TryInto<Vec<u8>> for Variant {
    fn try_into(self) -> Result<Vec<u8>> {
        match self {
            Variant::Buffer(val) => Ok(val.into_vec()),
            Variant::Slice(val) => Ok(val.as_ref().to_vec()),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<ByteSlice> for Variant {
    fn try_into(self) -> Result<ByteSlice> {
        match self {
            Variant::Slice(val) => Ok(val),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<u64> for Variant {
    fn try_into(self) -> Result<u64> {
        TryInto::<u128>::try_into(self).map(|v| v as u64)
    }
}

impl TryInto<u128> for Variant {
    fn try_into(self) -> Result<u128> {
        match self {
            Variant::Int64(val) => Ok(val as u128),
            Variant::UInt64(val) => Ok(u128::from(val)),
            Variant::Float64(val) => Ok(val as u128),
            Variant::BigInt(val) => val.to_u128().ok_or_else(|| err_msg("overflow")),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<bool> for Variant {
    fn try_into(self) -> Result<bool> {
        match self {
            Variant::Bool(val) => Ok(val),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<usize> for Variant {
    fn try_into(self) -> Result<usize> {
        TryInto::<u64>::try_into(self).map(|v| v as usize)
    }
}

impl TryInto<u32> for Variant {
    fn try_into(self) -> Result<u32> {
        TryInto::<u64>::try_into(self).map(|v| v as u32)
    }
}

impl TryInto<u16> for Variant {
    fn try_into(self) -> Result<u16> {
        TryInto::<u64>::try_into(self).map(|v| v as u16)
    }
}

impl TryInto<u8> for Variant {
    fn try_into(self) -> Result<u8> {
        TryInto::<u64>::try_into(self).map(|v| v as u8)
    }
}

impl TryInto<i64> for Variant {
    fn try_into(self) -> Result<i64> {
        TryInto::<i128>::try_into(self).map(|v| v as i64)
    }
}

impl TryInto<i128> for Variant {
    fn try_into(self) -> Result<i128> {
        match self {
            Variant::Int64(val) => Ok(i128::from(val)),
            Variant::UInt64(val) => Ok(i128::from(val)),
            Variant::Float64(val) => Ok(val as i128),
            Variant::BigInt(val) => val.to_i128().ok_or_else(|| err_msg("overflow")),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<isize> for Variant {
    fn try_into(self) -> Result<isize> {
        TryInto::<i64>::try_into(self).map(|v| v as isize)
    }
}

impl TryInto<i32> for Variant {
    fn try_into(self) -> Result<i32> {
        TryInto::<i64>::try_into(self).map(|v| v as i32)
    }
}

impl TryInto<i16> for Variant {
    fn try_into(self) -> Result<i16> {
        TryInto::<i64>::try_into(self).map(|v| v as i16)
    }
}

impl TryInto<i8> for Variant {
    fn try_into(self) -> Result<i8> {
        TryInto::<i64>::try_into(self).map(|v| v as i8)
    }
}

impl TryInto<f64> for Variant {
    fn try_into(self) -> Result<f64> {
        match self {
            Variant::Int64(val) => Ok(val as f64),
            Variant::UInt64(val) => Ok(val as f64),
            Variant::Float64(val) => Ok(val as f64),
            _ => Err(err_msg("wrong type")),
        }
    }
}

impl TryInto<f32> for Variant {
    fn try_into(self) -> Result<f32> {
        TryInto::<f64>::try_into(self).map(|v| v as f32)
    }
}

impl Into<Variant> for bool {
    fn into(self) -> Variant {
        Variant::Bool(self)
    }
}

impl Into<Variant> for i8 {
    fn into(self) -> Variant {
        Variant::Int64(i64::from(self))
    }
}

impl Into<Variant> for i16 {
    fn into(self) -> Variant {
        Variant::Int64(i64::from(self))
    }
}

impl Into<Variant> for i32 {
    fn into(self) -> Variant {
        Variant::Int64(i64::from(self))
    }
}

impl Into<Variant> for i64 {
    fn into(self) -> Variant {
        Variant::Int64(self)
    }
}

impl Into<Variant> for i128 {
    fn into(self) -> Variant {
        Variant::BigInt(self.into())
    }
}

impl Into<Variant> for u8 {
    fn into(self) -> Variant {
        Variant::UInt64(u64::from(self))
    }
}

impl Into<Variant> for u16 {
    fn into(self) -> Variant {
        Variant::UInt64(u64::from(self))
    }
}

impl Into<Variant> for u32 {
    fn into(self) -> Variant {
        Variant::UInt64(u64::from(self))
    }
}

impl Into<Variant> for u64 {
    fn into(self) -> Variant {
        Variant::UInt64(self)
    }
}

impl Into<Variant> for u128 {
    fn into(self) -> Variant {
        Variant::BigInt(self.into())
    }
}

impl Into<Variant> for f32 {
    fn into(self) -> Variant {
        Variant::Float64(f64::from(self))
    }
}

impl Into<Variant> for f64 {
    fn into(self) -> Variant {
        Variant::Float64(self)
    }
}

impl Into<Variant> for Box<[u8]> {
    fn into(self) -> Variant {
        Variant::Buffer(self)
    }
}

impl Into<Variant> for ByteSlice {
    fn into(self) -> Variant {
        Variant::Slice(self)
    }
}

impl<T: Into<Variant>> From<Option<T>> for Variant {
    fn from(v: Option<T>) -> Self {
        match v {
            Some(v) => v.into(),
            None => Variant::Nil,
        }
    }
}
