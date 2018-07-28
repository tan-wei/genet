//! Fixed-lifetime shareable containers.

use std::{
    fmt,
    ops::{Deref, DerefMut},
};

/// A fixed memory location.
#[repr(C)]
#[derive(Copy)]
pub struct Fixed<T> {
    ptr: *const T,
}

unsafe impl<T: Send> Send for Fixed<T> {}
unsafe impl<T: Sync> Sync for Fixed<T> {}

impl<T> fmt::Debug for Fixed<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Fixed {:?}", self.ptr)
    }
}

impl<T> Clone for Fixed<T> {
    fn clone(&self) -> Fixed<T> {
        Self { ptr: self.ptr }
    }
}

impl<T> Fixed<T> {
    pub fn new(data: T) -> Fixed<T> {
        Self {
            ptr: Box::into_raw(Box::new(data)),
        }
    }

    pub fn from_box(data: Box<T>) -> Fixed<T> {
        Self {
            ptr: Box::into_raw(data),
        }
    }

    pub fn from_static(data: &'static T) -> Fixed<T> {
        Self { ptr: data }
    }

    pub fn as_ptr(&self) -> *const T {
        self.ptr
    }
}

impl<T, D: Deref<Target = T>> From<&'static D> for Fixed<T> {
    fn from(data: &'static D) -> Fixed<T> {
        Fixed { ptr: data.deref() }
    }
}

impl<T> AsRef<T> for Fixed<T> {
    fn as_ref(&self) -> &T {
        unsafe { &*self.ptr }
    }
}

impl<T> Deref for Fixed<T> {
    type Target = T;

    fn deref(&self) -> &T {
        self.as_ref()
    }
}

/// A mutable fixed memory location.
#[repr(C)]
pub struct MutFixed<T> {
    ptr: *mut T,
}

unsafe impl<T: Send> Send for MutFixed<T> {}

impl<T> fmt::Debug for MutFixed<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "MutFixed {:?}", self.ptr)
    }
}

impl<T> MutFixed<T> {
    pub fn new(data: T) -> MutFixed<T> {
        Self {
            ptr: Box::into_raw(Box::new(data)),
        }
    }

    pub fn as_ptr(&self) -> *const T {
        self.ptr
    }

    pub fn as_mut_ptr(&self) -> *mut T {
        self.ptr
    }
}

impl<T> Deref for MutFixed<T> {
    type Target = T;

    fn deref(&self) -> &T {
        unsafe { &*self.ptr }
    }
}

impl<T> DerefMut for MutFixed<T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.ptr }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new() {
        let data = 123u32;
        let ptr = Fixed::new(data);
        assert_eq!(*ptr, data);
    }
}
