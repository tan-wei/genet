//! Byte sequence reader.
//!
//! Type Reader provides convenient functions to reading numbers and slices from a byte sequence.

extern crate byteorder;

use std::mem;
use std::io::{Cursor, Error, ErrorKind, Result};
use self::byteorder::{ByteOrder, ReadBytesExt};
use super::range::Range;

pub trait ByteReader<T> {
    fn read_i8(&mut self) -> Result<(i32, Range)>;
    fn read_i16<B: ByteOrder>(&mut self) -> Result<(i32, Range)>;
    fn read_i32<B: ByteOrder>(&mut self) -> Result<(i32, Range)>;
    fn read_u8(&mut self) -> Result<(u32, Range)>;
    fn read_u16<B: ByteOrder>(&mut self) -> Result<(u32, Range)>;
    fn read_u32<B: ByteOrder>(&mut self) -> Result<(u32, Range)>;
    fn read_f32<B: ByteOrder>(&mut self) -> Result<(f64, Range)>;
    fn read_f64<B: ByteOrder>(&mut self) -> Result<(f64, Range)>;
    fn read_slice(&mut self, len: usize) -> Result<(&'static [u8], Range)>;
    fn read_slice_to_end(&mut self) -> Result<(&'static [u8], Range)>;
}

impl ByteReader<&'static [u8]> for Cursor<&'static [u8]> {
    fn read_i8(&mut self) -> Result<(i32, Range)> {
        ReadBytesExt::read_i8(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((i32::from(val), range))
        })
    }

    fn read_i16<B: ByteOrder>(&mut self) -> Result<(i32, Range)> {
        ReadBytesExt::read_i16::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((i32::from(val), range))
        })
    }

    fn read_i32<B: ByteOrder>(&mut self) -> Result<(i32, Range)> {
        ReadBytesExt::read_i32::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((val as i32, range))
        })
    }

    fn read_u8(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u8(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((u32::from(val), range))
        })
    }

    fn read_u16<B: ByteOrder>(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u16::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((u32::from(val), range))
        })
    }

    fn read_u32<B: ByteOrder>(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u32::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((val as u32, range))
        })
    }

    fn read_f32<B: ByteOrder>(&mut self) -> Result<(f64, Range)> {
        ReadBytesExt::read_f32::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((f64::from(val), range))
        })
    }

    fn read_f64<B: ByteOrder>(&mut self) -> Result<(f64, Range)> {
        ReadBytesExt::read_f64::<B>(self).and_then(|val| {
            let range = self.position() as u32 - mem::size_of_val(&val) as u32..self.position() as u32;
            Ok((val as f64, range))
        })
    }

    fn read_slice(&mut self, len: usize) -> Result<(&'static [u8], Range)> {
        let begin = self.position();
        let end = begin + len as u64;
        let max = self.get_ref().len() as u64;
        if end > max {
            Err(Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
        } else {
            {
                self.set_position(end as u64)
            }
            Ok((&self.get_ref()[begin as usize..end as usize], begin as u32..end as u32))
        }
    }

    fn read_slice_to_end(&mut self) -> Result<(&'static [u8], Range)> {
        let begin = self.position();
        let max = self.get_ref().len();
        self.read_slice(max - begin as usize)
    }
}
