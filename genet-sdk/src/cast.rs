//! Cast objects for the usual data structures.

use crate::{attr::Attr, slice};
use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
pub use genet_abi::cast::{Cast, Map, Nil, Typed};
use genet_abi::slice::TryGet;
use std::{
    io::{Cursor, Error, ErrorKind, Result},
    mem::size_of,
};

/// Cast for 8bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt8();

impl Typed for UInt8 {
    type Output = u8;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u8> {
        Cursor::new(data.try_get(attr.range())?).read_u8()
    }

    fn bit_size(&self) -> usize {
        size_of::<u8>() * 8
    }
}

/// Cast for 8bit signed integer.
#[derive(Clone, Default)]
pub struct Int8();

impl Typed for Int8 {
    type Output = i8;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i8> {
        Cursor::new(data.try_get(attr.range())?).read_i8()
    }

    fn bit_size(&self) -> usize {
        size_of::<i8>() * 8
    }
}

/// Cast for big-endian 16bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt16BE();

impl Typed for UInt16BE {
    type Output = u16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u16> {
        Cursor::new(data.try_get(attr.range())?).read_u16::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u16>() * 8
    }
}

/// Cast for big-endian 32bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt32BE();

impl Typed for UInt32BE {
    type Output = u32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u32> {
        Cursor::new(data.try_get(attr.range())?).read_u32::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u32>() * 8
    }
}

/// Cast for big-endian 64bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt64BE();

impl Typed for UInt64BE {
    type Output = u64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u64> {
        Cursor::new(data.try_get(attr.range())?).read_u64::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u64>() * 8
    }
}

/// Cast for big-endian 16bit signed integer.
#[derive(Clone, Default)]
pub struct Int16BE();

impl Typed for Int16BE {
    type Output = i16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i16> {
        Cursor::new(data.try_get(attr.range())?).read_i16::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i16>() * 8
    }
}

/// Cast for big-endian 32bit signed integer.
#[derive(Clone, Default)]
pub struct Int32BE();

impl Typed for Int32BE {
    type Output = i32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i32> {
        Cursor::new(data.try_get(attr.range())?).read_i32::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i32>() * 8
    }
}

/// Cast for big-endian 64bit signed integer.
#[derive(Clone, Default)]
pub struct Int64BE();

impl Typed for Int64BE {
    type Output = i64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i64> {
        Cursor::new(data.try_get(attr.range())?).read_i64::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i64>() * 8
    }
}

/// Cast for big-endian 32bit floating point number.
#[derive(Clone, Default)]
pub struct Float32BE();

impl Typed for Float32BE {
    type Output = f32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f32> {
        Cursor::new(data.try_get(attr.range())?).read_f32::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<f32>() * 8
    }
}

/// Cast for big-endian 64bit floating point number.
#[derive(Clone, Default)]
pub struct Float64BE();

impl Typed for Float64BE {
    type Output = f64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f64> {
        Cursor::new(data.try_get(attr.range())?).read_f64::<BigEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<f64>() * 8
    }
}

/// Cast for little-endian 16bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt16LE();

impl Typed for UInt16LE {
    type Output = u16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u16> {
        Cursor::new(data.try_get(attr.range())?).read_u16::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u16>() * 8
    }
}

/// Cast for little-endian 32bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt32LE();

impl Typed for UInt32LE {
    type Output = u32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u32> {
        Cursor::new(data.try_get(attr.range())?).read_u32::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u32>() * 8
    }
}

/// Cast for little-endian 64bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt64LE();

impl Typed for UInt64LE {
    type Output = u64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u64> {
        Cursor::new(data.try_get(attr.range())?).read_u64::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<u64>() * 8
    }
}

/// Cast for little-endian 16bit signed integer.
#[derive(Clone, Default)]
pub struct Int16LE();

impl Typed for Int16LE {
    type Output = i16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i16> {
        Cursor::new(data.try_get(attr.range())?).read_i16::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i16>() * 8
    }
}

/// Cast for little-endian 32bit signed integer.
#[derive(Clone, Default)]
pub struct Int32LE();

impl Typed for Int32LE {
    type Output = i32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i32> {
        Cursor::new(data.try_get(attr.range())?).read_i32::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i32>() * 8
    }
}

/// Cast for little-endian 64bit signed integer.
#[derive(Clone, Default)]
pub struct Int64LE();

impl Typed for Int64LE {
    type Output = i64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i64> {
        Cursor::new(data.try_get(attr.range())?).read_i64::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<i64>() * 8
    }
}

/// Cast for little-endian 32bit floating point number.
#[derive(Clone, Default)]
pub struct Float32LE();

impl Typed for Float32LE {
    type Output = f32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f32> {
        Cursor::new(data.try_get(attr.range())?).read_f32::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<f32>() * 8
    }
}

/// Cast for little-endian 64bit floating point number.
#[derive(Clone, Default)]
pub struct Float64LE();

impl Typed for Float64LE {
    type Output = f64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f64> {
        Cursor::new(data.try_get(attr.range())?).read_f64::<LittleEndian>()
    }

    fn bit_size(&self) -> usize {
        size_of::<f64>() * 8
    }
}

/// Cast for UTF-8 string.
#[derive(Clone, Default)]
pub struct Utf8();

impl Typed for Utf8 {
    type Output = Box<str>;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Box<str>> {
        if let Ok(s) = String::from_utf8(data.try_get(attr.range())?.to_vec()) {
            Ok(s.into_boxed_str())
        } else {
            Err(Error::new(ErrorKind::InvalidData, "Invalid UTF-8"))
        }
    }
}

/// Cast for ByteSlice.
#[derive(Clone, Default)]
pub struct ByteSlice();

impl Typed for ByteSlice {
    type Output = slice::ByteSlice;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<slice::ByteSlice> {
        Ok(data.try_get(attr.range())?)
    }
}

#[derive(Clone, Default)]
pub struct BitFlag();

impl Typed for BitFlag {
    type Output = bool;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<bool> {
        let byte = Cursor::new(data.try_get(attr.range())?).read_u8()?;
        Ok((byte & (0b1000_0000 >> (attr.bit_range().start % 8))) != 0)
    }

    fn bit_size(&self) -> usize {
        1
    }
}
