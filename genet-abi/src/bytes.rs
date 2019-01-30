use crate::result::Result;
use failure::err_msg;
use std::{
    mem,
    ops::{Deref, Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
    slice,
};

/// TryGet trait.
pub trait TryGet<T> {
    type Output;

    /// Returns a byte or subslice depending on the type of index.
    fn get(&self, index: T) -> Result<Self::Output>;
}

macro_rules! impl_slice_index {
    ( $( $x:ty ), * ) => {
        $(
            impl TryGet<$x> for Bytes {
                type Output = Bytes;

                fn get(&self, index: $x) -> Result<Bytes> {
                    <[u8]>::get(self, index)
                        .map(|s| unsafe { Bytes::from_raw_parts(s.as_ptr(), s.len()) })
                        .ok_or_else(|| err_msg("out of bounds"))
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

impl TryGet<usize> for Bytes {
    type Output = u8;

    fn get(&self, index: usize) -> Result<u8> {
        <[u8]>::get(self, index)
            .cloned()
            .ok_or_else(|| err_msg("out of bounds"))
    }
}

/// A fixed-lifetime slice object.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct Bytes(&'static [u8]);

impl Bytes {
    /// Creates a new empty Bytes.
    pub fn new() -> Bytes {
        Bytes(&[])
    }

    /// Creates a new Bytes from a length and pointer.
    ///
    /// The pointer must be valid during the program execution.
    pub unsafe fn from_raw_parts(data: *const u8, len: usize) -> Bytes {
        Bytes(slice::from_raw_parts(data, len))
    }

    /// Returns the length of this Bytes.
    pub fn len(&self) -> usize {
        self.0.len()
    }

    /// Returns true if this Bytes has a length of zero.
    ///
    /// Returns false otherwise.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Returns a raw pointer to the first byte in this Bytes.
    pub fn as_ptr(&self) -> *const u8 {
        self.0.as_ptr()
    }

    pub fn as_slice(&self) -> &'static [u8] {
        self.0
    }
}

impl From<&'static [u8]> for Bytes {
    fn from(data: &'static [u8]) -> Self {
        Bytes(data)
    }
}

impl From<Box<[u8]>> for Bytes {
    fn from(data: Box<[u8]>) -> Self {
        let s = unsafe { Bytes::from_raw_parts(data.as_ptr(), data.len()) };
        mem::forget(data);
        s
    }
}

impl From<Vec<u8>> for Bytes {
    fn from(data: Vec<u8>) -> Self {
        Bytes::from(data.into_boxed_slice())
    }
}

impl Deref for Bytes {
    type Target = [u8];

    fn deref(&self) -> &'static [u8] {
        self.0
    }
}

impl AsRef<[u8]> for Bytes {
    #[inline]
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}
