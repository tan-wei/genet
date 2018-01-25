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
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as i32, range))
        })
    }

    fn read_i16<B: ByteOrder>(&mut self) -> Result<(i32, Range)> {
        ReadBytesExt::read_i16::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as i32, range))
        })
    }

    fn read_i32<B: ByteOrder>(&mut self) -> Result<(i32, Range)> {
        ReadBytesExt::read_i32::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as i32, range))
        })
    }

    fn read_u8(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u8(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as u32, range))
        })
    }

    fn read_u16<B: ByteOrder>(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u16::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as u32, range))
        })
    }

    fn read_u32<B: ByteOrder>(&mut self) -> Result<(u32, Range)> {
        ReadBytesExt::read_u32::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as u32, range))
        })
    }

    fn read_f32<B: ByteOrder>(&mut self) -> Result<(f64, Range)> {
        ReadBytesExt::read_f32::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as f64, range))
        })
    }

    fn read_f64<B: ByteOrder>(&mut self) -> Result<(f64, Range)> {
        ReadBytesExt::read_f64::<B>(self).and_then(|val| {
            let range = self.position() as usize..self.position() as usize + mem::size_of_val(&val);
            Ok((val as f64, range))
        })
    }

    fn read_slice(&mut self, len: usize) -> Result<(&'static [u8], Range)> {
        let begin = self.position() as usize;
        let end = begin + len;
        let max = self.get_ref().len();
        if end > max {
            Err(Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
        } else {
            {
                self.set_position(end as u64)
            }
            Ok((&self.get_ref()[begin..end], begin..end))
        }
    }

    fn read_slice_to_end(&mut self) -> Result<(&'static [u8], Range)> {
        let begin = self.position() as usize;
        let max = self.get_ref().len();
        self.read_slice(max - begin)
    }
}
