use std::{
    fmt,
    ops::{Deref, DerefMut},
};

#[repr(C)]
#[derive(Copy)]
pub struct Ptr<T> {
    ptr: *const T,
}

unsafe impl<T: Send> Send for Ptr<T> {}
unsafe impl<T: Sync> Sync for Ptr<T> {}

impl<T> fmt::Debug for Ptr<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Ptr {:?}", self.ptr)
    }
}

impl<T> Clone for Ptr<T> {
    fn clone(&self) -> Ptr<T> {
        Self { ptr: self.ptr }
    }
}

impl<T> Ptr<T> {
    pub fn new(data: T) -> Ptr<T> {
        Self {
            ptr: Box::into_raw(Box::new(data)),
        }
    }

    pub fn from_box(data: Box<T>) -> Ptr<T> {
        Self {
            ptr: Box::into_raw(data),
        }
    }

    pub fn as_ptr(&self) -> *const T {
        self.ptr
    }
}

impl<T, D: Deref<Target = T>> From<&'static D> for Ptr<T> {
    fn from(data: &'static D) -> Ptr<T> {
        Ptr { ptr: data.deref() }
    }
}

impl<T> AsRef<T> for Ptr<T> {
    fn as_ref(&self) -> &T {
        unsafe { &*self.ptr }
    }
}

impl<T> Deref for Ptr<T> {
    type Target = T;

    fn deref(&self) -> &T {
        self.as_ref()
    }
}

#[repr(C)]
pub struct MutPtr<T> {
    ptr: *mut T,
}

unsafe impl<T: Send> Send for MutPtr<T> {}

impl<T> fmt::Debug for MutPtr<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "MutPtr {:?}", self.ptr)
    }
}

impl<T> MutPtr<T> {
    pub fn new(data: T) -> MutPtr<T> {
        Self {
            ptr: Box::into_raw(Box::new(data)),
        }
    }

    pub fn from_box(data: Box<T>) -> MutPtr<T> {
        Self {
            ptr: Box::into_raw(data),
        }
    }

    pub fn as_ptr(&self) -> *const T {
        self.ptr
    }

    pub fn as_mut_ptr(&self) -> *mut T {
        self.ptr
    }
}

impl<T> Deref for MutPtr<T> {
    type Target = T;

    fn deref(&self) -> &T {
        unsafe { &*self.ptr }
    }
}

impl<T> DerefMut for MutPtr<T> {
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
        let ptr = Ptr::new(data);
        assert_eq!(*ptr, data);
    }

    #[test]
    fn from_box() {
        let data = 123u32;
        let ptr = Ptr::from_box(Box::new(data));
        assert_eq!(*ptr, data);
    }
}
