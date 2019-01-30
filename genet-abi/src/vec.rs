use std::{
    iter::{FromIterator, IntoIterator},
    mem,
    ops::{Deref, DerefMut},
    ptr, slice,
};

#[repr(C)]
#[derive(Debug)]
pub struct IntoIter<T>
where
    T: Sized,
{
    ptr: *mut T,
    len: u64,
    cap: u64,
    offset: u64,
}

impl<T> Iterator for IntoIter<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if self.len > 0 {
            unsafe {
                let val = Some(ptr::read(self.ptr.add(self.offset as usize)));
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
        unsafe {
            Vec::from_raw_parts(self.ptr, self.len as usize, self.cap as usize);
        }
    }
}

#[repr(C)]
#[derive(Debug)]
pub struct SafeVec<T>
where
    T: Sized,
{
    ptr: *mut T,
    len: u64,
    cap: u64,
}

impl<T> SafeVec<T> {
    pub fn new() -> Self {
        let mut vec = Vec::new();
        let ptr = vec.as_mut_ptr();
        let len = vec.len() as u64;
        let cap = vec.capacity() as u64;
        mem::forget(vec);
        SafeVec { ptr, len, cap }
    }

    pub fn with_capacity(capacity: u64) -> Self {
        let mut vec = Vec::with_capacity(capacity as usize);
        let ptr = vec.as_mut_ptr();
        let len = vec.len() as u64;
        let cap = vec.capacity() as u64;
        mem::forget(vec);
        SafeVec { ptr, len, cap }
    }

    pub fn push(&mut self, val: T) {
        unsafe {
            if self.cap <= self.len {
                let mut vec = Vec::from_raw_parts(self.ptr, self.len as usize, self.cap as usize);
                vec.reserve((self.len - self.cap) as usize + 1);
                self.ptr = vec.as_mut_ptr();
                self.cap = vec.capacity() as u64;
                mem::forget(vec);
            }
            let ptr = self.ptr.add(self.len as usize);
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
    type IntoIter = self::IntoIter<T>;
    type Item = T;

    fn into_iter(mut self) -> Self::IntoIter {
        let iter = IntoIter {
            ptr: self.ptr,
            len: self.len,
            cap: self.cap,
            offset: 0,
        };
        unsafe { ptr::write(&mut self, SafeVec::new()) };
        iter
    }
}

impl<T> FromIterator<T> for SafeVec<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> SafeVec<T> {
        let mut v = SafeVec::new();
        let iter = iter.into_iter();
        for val in iter {
            v.push(val);
        }
        v
    }
}

unsafe impl<T: Send> Send for SafeVec<T> {}

impl<T> Drop for SafeVec<T> {
    fn drop(&mut self) {
        unsafe {
            Vec::from_raw_parts(self.ptr, self.len as usize, self.cap as usize);
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
        let mut v = SafeVec::with_capacity(s.len() as u64);
        for i in s {
            v.push(i.clone());
        }
        v
    }
}

impl<T: Clone> From<Vec<T>> for SafeVec<T> {
    fn from(s: Vec<T>) -> SafeVec<T> {
        s.as_slice().into()
    }
}

impl<'a, T: Clone> From<&'a Box<[T]>> for SafeVec<T> {
    fn from(s: &'a Box<[T]>) -> SafeVec<T> {
        s.as_ref().into()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn push() {
        let mut v = SafeVec::<u64>::new();
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
        let mut v = SafeVec::<u64>::new();
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
}
