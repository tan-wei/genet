extern crate bytes;

pub use self::bytes::Bytes;
use std::{
    io::{Error, ErrorKind, Result},
    ops::{Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
};

pub trait TryGet<T> {
    type Output;
    fn try_get(&self, index: T) -> Result<Self::Output>;
}

impl TryGet<usize> for Bytes {
    type Output = u8;

    fn try_get(&self, index: usize) -> Result<u8> {
        <[u8]>::get(self, index)
            .cloned()
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<Range<usize>> for Bytes {
    type Output = Bytes;

    fn try_get(&self, index: Range<usize>) -> Result<Bytes> {
        <[u8]>::get(self, index.clone())
            .map(|_| self.slice(index.start, index.end))
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<RangeInclusive<usize>> for Bytes {
    type Output = Bytes;

    fn try_get(&self, index: RangeInclusive<usize>) -> Result<Bytes> {
        <[u8]>::get(self, index.clone())
            .map(|_| self.slice(*index.start(), index.end() + 1))
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<RangeTo<usize>> for Bytes {
    type Output = Bytes;

    fn try_get(&self, index: RangeTo<usize>) -> Result<Bytes> {
        <[u8]>::get(self, index)
            .map(|_| self.slice_to(index.end))
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<RangeToInclusive<usize>> for Bytes {
    type Output = Bytes;

    fn try_get(&self, index: RangeToInclusive<usize>) -> Result<Bytes> {
        <[u8]>::get(self, index)
            .map(|_| self.slice_to(index.end))
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<RangeFrom<usize>> for Bytes {
    type Output = Bytes;

    fn try_get(&self, index: RangeFrom<usize>) -> Result<Bytes> {
        <[u8]>::get(self, index.clone())
            .map(|_| self.slice_from(index.start))
            .ok_or_else(|| Error::new(ErrorKind::UnexpectedEof, "out of bounds"))
    }
}

impl TryGet<RangeFull> for Bytes {
    type Output = Bytes;

    fn try_get(&self, _index: RangeFull) -> Result<Bytes> {
        Ok(self.clone())
    }
}
