use std::{
    io::{Error, ErrorKind, Result},
    mem,
    ops::{Deref, Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
    ptr, slice,
};

/// TryGet trait.
pub trait TryGet<T> {
    type Output;

    /// Returns a byte or subslice depending on the type of index.
    fn try_get(&self, index: T) -> Result<Self::Output>;
}

macro_rules! impl_slice_index {
    ( $( $x:ty ), * ) => {
        $(
            impl TryGet<$x> for ByteSlice {
                type Output = ByteSlice;

                fn try_get(&self, index: $x) -> Result<ByteSlice> {
                    <[u8]>::get(self, index)
                        .map(|s| unsafe { ByteSlice::from_raw_parts(s.as_ptr(), s.len()) })
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

impl TryGet<usize> for ByteSlice {
    type Output = u8;

    fn try_get(&self, index: usize) -> Result<u8> {
        <[u8]>::get(self, index)
            .cloned()
            .ok_or_else(|| Error::new(ErrorKind::Other, "out of bounds"))
    }
}

/// A fixed-lifetime slice object.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct ByteSlice {
    data: *const u8,
    len: u64,
}

unsafe impl Send for ByteSlice {}
unsafe impl Sync for ByteSlice {}

impl ByteSlice {
    /// Creates a new empty ByteSlice.
    pub fn new() -> ByteSlice {
        ByteSlice {
            data: ptr::null(),
            len: 0,
        }
    }

    /// Creates a new ByteSlice from a length and pointer.
    ///
    /// The pointer must be valid during the program execution.
    pub unsafe fn from_raw_parts(data: *const u8, len: usize) -> ByteSlice {
        ByteSlice {
            data,
            len: len as u64,
        }
    }

    /// Returns the length of this ByteSlice.
    pub fn len(&self) -> usize {
        self.len as usize
    }

    /// Returns true if this ByteSlice has a length of zero.
    ///
    /// Returns false otherwise.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Returns a raw pointer to the first byte in this ByteSlice.
    pub fn as_ptr(&self) -> *const u8 {
        self.data
    }
}

impl From<&'static [u8]> for ByteSlice {
    fn from(data: &'static [u8]) -> Self {
        ByteSlice {
            data: data.as_ptr(),
            len: data.len() as u64,
        }
    }
}

impl From<Box<[u8]>> for ByteSlice {
    fn from(data: Box<[u8]>) -> Self {
        let s = ByteSlice {
            data: data.as_ptr(),
            len: data.len() as u64,
        };
        mem::forget(data);
        s
    }
}

impl From<Vec<u8>> for ByteSlice {
    fn from(data: Vec<u8>) -> Self {
        ByteSlice::from(data.into_boxed_slice())
    }
}

impl Deref for ByteSlice {
    type Target = [u8];

    fn deref(&self) -> &'static [u8] {
        unsafe { slice::from_raw_parts(self.data, self.len as usize) }
    }
}

impl AsRef<[u8]> for ByteSlice {
    #[inline]
    fn as_ref(&self) -> &[u8] {
        &self.deref()
    }
}
