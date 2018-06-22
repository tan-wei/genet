use env;
use std::iter::{FromIterator, IntoIterator};
use std::mem;
use std::ops::{Deref, DerefMut};
use std::ptr;
use std::slice;

#[repr(C)]
#[derive(Debug)]
pub struct IntoIter<T>
where
    T: Sized,
{
    ptr: *mut T,
    len: u32,
    offset: u32,
}

impl<T> Iterator for IntoIter<T> {
    type Item = T;
    fn next(&mut self) -> Option<Self::Item> {
        if self.len > 0 {
            unsafe {
                let val = Some(ptr::read(self.ptr.offset(self.offset as isize)));
                self.offset += 1;
                self.len -= 1;
                val
            }
        } else {
            None
        }
    }
}

impl<T> Drop for IntoIter<T> {
    fn drop(&mut self) {
        env::dealloc(self.ptr as *mut u8);
    }
}

#[repr(C)]
#[derive(Debug)]
pub struct SafeVec<T>
where
    T: Sized,
{
    ptr: *mut T,
    len: u32,
    cap: u32,
}

impl<T> SafeVec<T> {
    pub fn new() -> Self {
        Self {
            ptr: ptr::null_mut(),
            len: 0,
            cap: 0,
        }
    }

    pub fn with_capacity(capacity: u32) -> Self {
        let size = mem::size_of::<T>() * capacity as usize;
        let ptr: *mut T = unsafe { mem::transmute(env::alloc(size)) };
        if ptr.is_null() {
            panic!("alloc() returns NULL");
        }
        SafeVec {
            ptr,
            len: 0,
            cap: capacity,
        }
    }

    pub fn into_raw(mut self) -> (*mut T, u32) {
        let ptr = self.ptr;
        self.ptr = ptr::null_mut();
        self.cap = 0;
        (ptr, self.len)
    }

    pub fn push(&mut self, val: T) {
        unsafe {
            if self.cap <= self.len {
                self.cap += 4;
                let size = mem::size_of::<T>() * self.cap as usize;
                self.ptr = mem::transmute(env::realloc(self.ptr as *mut u8, size));
                if self.ptr.is_null() {
                    panic!("realloc() returns NULL");
                }
            }
            let ptr = self.ptr.offset(self.len as isize);
            ptr::write(&mut *ptr, val);
            self.len += 1;
        }
    }

    pub fn as_slice(&self) -> &[T] {
        unsafe { slice::from_raw_parts(&*self.ptr, self.len as usize) }
    }

    pub fn as_mut_slice(&mut self) -> &mut [T] {
        unsafe { slice::from_raw_parts_mut(&mut *self.ptr, self.len as usize) }
    }
}

impl<T> IntoIterator for SafeVec<T> {
    type Item = T;
    type IntoIter = self::IntoIter<T>;

    fn into_iter(mut self) -> Self::IntoIter {
        let iter = IntoIter {
            ptr: self.ptr,
            len: self.len,
            offset: 0,
        };
        self.len = 0;
        self.cap = 0;
        self.ptr = ptr::null_mut();
        iter
    }
}

impl<T> FromIterator<T> for SafeVec<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> SafeVec<T> {
        let mut v = SafeVec::new();
        let mut iter = iter.into_iter();
        while let Some(val) = iter.next() {
            v.push(val);
        }
        v
    }
}

unsafe impl<T: Send> Send for SafeVec<T> {}

impl<T> Drop for SafeVec<T> {
    fn drop(&mut self) {
        unsafe {
            for i in 0..self.len {
                ptr::drop_in_place(self.ptr.offset(i as isize));
            }
            env::dealloc(self.ptr as *mut u8);
        }
    }
}

impl<T> Deref for SafeVec<T> {
    type Target = [T];

    fn deref(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T> DerefMut for SafeVec<T> {
    fn deref_mut(&mut self) -> &mut [T] {
        self.as_mut_slice()
    }
}

impl<T: Clone> Clone for SafeVec<T> {
    fn clone(&self) -> Self {
        let mut v = SafeVec::with_capacity(self.len);
        for i in self.iter() {
            v.push(i.clone());
        }
        v
    }
}

impl<'a, T: Clone> From<&'a [T]> for SafeVec<T> {
    fn from(s: &'a [T]) -> Self {
        let mut v = SafeVec::with_capacity(s.len() as u32);
        for i in s {
            v.push(i.clone());
        }
        v
    }
}

impl<T: Clone> From<Box<[T]>> for SafeVec<T> {
    fn from(s: Box<[T]>) -> SafeVec<T> {
        SafeVec::<T>::from(&*s)
    }
}

impl<'a, T: Clone> From<&'a Box<[T]>> for SafeVec<T> {
    fn from(s: &'a Box<[T]>) -> SafeVec<T> {
        let mut v = SafeVec::with_capacity(s.len() as u32);
        for i in s.iter() {
            v.push(i.clone());
        }
        v
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn push() {
        let mut v = SafeVec::<u32>::new();
        assert_eq!(v.first(), None);
        v.push(5);
        assert_eq!(v.first(), Some(&5));
        for i in 0..5000 {
            v.push(i);
        }
        assert_eq!(v.first(), Some(&5));
        assert_eq!(v.last(), Some(&4999));
    }

    #[test]
    fn clone() {
        let mut v = SafeVec::<u32>::new();
        for i in 0..5000 {
            v.push(i);
        }
        let v2 = v.clone();
        assert_eq!(v.len(), 5000);
        assert_eq!(v.first(), Some(&0));
        assert_eq!(v.last(), Some(&4999));
        assert_eq!(v2.len(), 5000);
        assert_eq!(v2.first(), Some(&0));
        assert_eq!(v2.last(), Some(&4999));
    }

    #[test]
    fn into_raw() {
        let mut v = SafeVec::<u32>::new();
        for i in 0..5000 {
            v.push(i);
        }
        let (ptr, len) = SafeVec::into_raw(v);
        assert_eq!(len, 5000);
        assert!(!ptr.is_null());
        env::dealloc(ptr as *mut u8);
    }
}
