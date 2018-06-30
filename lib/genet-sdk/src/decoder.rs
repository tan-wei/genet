use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
pub use genet_abi::decoder::{Decoder, Typed};
use slice;
use std::io::{Cursor, Error, ErrorKind, Result};

#[derive(Clone)]
pub struct UInt8();

impl Typed for UInt8 {
    type Output = u8;
    fn decode(&self, data: &slice::Slice) -> Result<u8> {
        Cursor::new(data).read_u8()
    }
}

#[derive(Clone)]
pub struct Int8();

impl Typed for Int8 {
    type Output = i8;
    fn decode(&self, data: &slice::Slice) -> Result<i8> {
        Cursor::new(data).read_i8()
    }
}

#[derive(Clone)]
pub struct UInt16BE();

impl Typed for UInt16BE {
    type Output = u16;
    fn decode(&self, data: &slice::Slice) -> Result<u16> {
        Cursor::new(data).read_u16::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct UInt32BE();

impl Typed for UInt32BE {
    type Output = u32;
    fn decode(&self, data: &slice::Slice) -> Result<u32> {
        Cursor::new(data).read_u32::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct UInt64BE();

impl Typed for UInt64BE {
    type Output = u64;
    fn decode(&self, data: &slice::Slice) -> Result<u64> {
        Cursor::new(data).read_u64::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct Int16BE();

impl Typed for Int16BE {
    type Output = i16;
    fn decode(&self, data: &slice::Slice) -> Result<i16> {
        Cursor::new(data).read_i16::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct Int32BE();

impl Typed for Int32BE {
    type Output = i32;
    fn decode(&self, data: &slice::Slice) -> Result<i32> {
        Cursor::new(data).read_i32::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct Int64BE();

impl Typed for Int64BE {
    type Output = i64;
    fn decode(&self, data: &slice::Slice) -> Result<i64> {
        Cursor::new(data).read_i64::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct Float32BE();

impl Typed for Float32BE {
    type Output = f32;
    fn decode(&self, data: &slice::Slice) -> Result<f32> {
        Cursor::new(data).read_f32::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct Float64BE();

impl Typed for Float64BE {
    type Output = f64;
    fn decode(&self, data: &slice::Slice) -> Result<f64> {
        Cursor::new(data).read_f64::<BigEndian>()
    }
}

#[derive(Clone)]
pub struct UInt16LE();

impl Typed for UInt16LE {
    type Output = u16;
    fn decode(&self, data: &slice::Slice) -> Result<u16> {
        Cursor::new(data).read_u16::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct UInt32LE();

impl Typed for UInt32LE {
    type Output = u32;
    fn decode(&self, data: &slice::Slice) -> Result<u32> {
        Cursor::new(data).read_u32::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct UInt64LE();

impl Typed for UInt64LE {
    type Output = u64;
    fn decode(&self, data: &slice::Slice) -> Result<u64> {
        Cursor::new(data).read_u64::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Int16LE();

impl Typed for Int16LE {
    type Output = i16;
    fn decode(&self, data: &slice::Slice) -> Result<i16> {
        Cursor::new(data).read_i16::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Int32LE();

impl Typed for Int32LE {
    type Output = i32;
    fn decode(&self, data: &slice::Slice) -> Result<i32> {
        Cursor::new(data).read_i32::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Int64LE();

impl Typed for Int64LE {
    type Output = i64;
    fn decode(&self, data: &slice::Slice) -> Result<i64> {
        Cursor::new(data).read_i64::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Float32LE();

impl Typed for Float32LE {
    type Output = f32;
    fn decode(&self, data: &slice::Slice) -> Result<f32> {
        Cursor::new(data).read_f32::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Float64LE();

impl Typed for Float64LE {
    type Output = f64;
    fn decode(&self, data: &slice::Slice) -> Result<f64> {
        Cursor::new(data).read_f64::<LittleEndian>()
    }
}

#[derive(Clone)]
pub struct Utf8();

impl Typed for Utf8 {
    type Output = Box<str>;
    fn decode(&self, data: &slice::Slice) -> Result<Box<str>> {
        if let Ok(s) = String::from_utf8(data.to_vec()) {
            Ok(s.into_boxed_str())
        } else {
            Err(Error::new(ErrorKind::InvalidData, "Invalid UTF-8"))
        }
    }
}

#[derive(Clone)]
pub struct Slice();

impl Typed for Slice {
    type Output = slice::Slice;
    fn decode(&self, data: &slice::Slice) -> Result<slice::Slice> {
        Ok(data.clone())
    }
}
