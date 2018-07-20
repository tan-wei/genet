use std::{
    io::{Error, ErrorKind, Result},
    marker::PhantomData,
    mem,
    ops::{Deref, Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
    slice,
};

pub trait TryGet<T> {
    type Output;

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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct ByteSlice(&'static [u8]);

impl ByteSlice {
    pub fn new() -> ByteSlice {
        ByteSlice(&[])
    }

    pub unsafe fn from_raw_parts(data: *const u8, len: usize) -> ByteSlice {
        ByteSlice(slice::from_raw_parts(data, len))
    }

    pub fn len(&self) -> usize {
        self.0.len()
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    pub fn as_ptr(&self) -> *const u8 {
        self.0.as_ptr()
    }
}

impl From<&'static [u8]> for ByteSlice {
    fn from(data: &'static [u8]) -> Self {
        ByteSlice(data)
    }
}

impl From<Box<[u8]>> for ByteSlice {
    fn from(data: Box<[u8]>) -> Self {
        let s = unsafe { ByteSlice::from_raw_parts(data.as_ptr(), data.len()) };
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
        self.0
    }
}

impl AsRef<[u8]> for ByteSlice {
    #[inline]
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

#[repr(C)]
struct SafeByteSlice<'a, T: 'a> {
    ptr: *const T,
    len: u64,
    phantom: PhantomData<&'a T>,
}

impl<'a, T: 'a> SafeByteSlice<'a, T> {
    pub fn new(data: &'a [T]) -> SafeByteSlice<'a, T> {
        Self {
            ptr: data.as_ptr(),
            len: data.len() as u64,
            phantom: PhantomData,
        }
    }

    pub fn as_slice(&self) -> &[T] {
        unsafe { slice::from_raw_parts(&*self.ptr, self.len as usize) }
    }
}

impl<'a, T: 'a> Deref for SafeByteSlice<'a, T> {
    type Target = [T];

    fn deref(&self) -> &[T] {
        self.as_slice()
    }
}
