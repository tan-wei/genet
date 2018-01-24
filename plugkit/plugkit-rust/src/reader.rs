extern crate byteorder;

use std::convert::AsRef;
use std::io::{Error,Cursor};
use self::byteorder::{ReadBytesExt, ByteOrder};
use super::range::Range;

pub trait ByteReader<B, T> {
  fn read_u16(&mut self) -> Result<(u16, Range), Error>;
}

impl<B, T> ByteReader<B, T> for Cursor<T> where T: AsRef<[u8]>, B: ByteOrder {
    fn read_u16(&mut self) -> Result<(u16, Range), Error> {
        let result = ReadBytesExt::read_u16::<B>(self);
        match { result } {
            Ok(val) => {
                let range = self.position() as usize..self.position() as usize + 2;
                Ok((val, range))
            },
            Err(e) => { Err(e) }
        }
    }
}
