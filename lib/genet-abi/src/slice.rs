use std::io::{Error, ErrorKind, Result};
use std::ops::{Deref, Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive};
use std::slice;

pub type Slice = &'static [u8];

pub trait SliceIndex<T> {
    fn get(&self, index: T) -> Result<SliceX>;
}

macro_rules! impl_slice_index {
    ( $( $x:ty ), * ) => {
        $(
            impl SliceIndex<$x> for SliceX {
                fn get(&self, index: $x) -> Result<SliceX> {
                    <[u8]>::get(self, index)
                        .map(|s| unsafe { SliceX::from_raw_parts(s.as_ptr(), s.len()) })
                        .ok_or_else(|| Error::new(ErrorKind::Other, "out of bounds"))
                }
            }
        )*
    };
}

impl_slice_index!(
    Range<usize>,
    RangeFrom<usize>,
    RangeFull,
    RangeInclusive<usize>,
    RangeTo<usize>,
    RangeToInclusive<usize>
);

pub struct SliceX(&'static [u8]);

impl SliceX {
    fn new(data: &'static [u8]) -> SliceX {
        SliceX(data)
    }

    unsafe fn from_raw_parts(data: *const u8, len: usize) -> SliceX {
        SliceX(unsafe { slice::from_raw_parts(data, len) })
    }

    fn len(self) -> usize {
        self.0.len()
    }

    fn as_ptr(self) -> *const u8 {
        self.0.as_ptr()
    }
}

impl From<Box<[u8]>> for SliceX {
    fn from(data: Box<[u8]>) -> Self {
        SliceX(&[])
    }
}

impl From<Vec<u8>> for SliceX {
    fn from(data: Vec<u8>) -> Self {
        SliceX::from(data.into_boxed_slice())
    }
}

impl Deref for SliceX {
    type Target = [u8];

    fn deref(&self) -> &'static [u8] {
        self.0
    }
}
