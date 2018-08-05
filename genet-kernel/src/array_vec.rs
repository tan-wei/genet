use std::{mem, ptr};

const BLOCK_SIZE: usize = 1024;

#[derive(Debug)]
pub struct ArrayVec<T>
where
    T: Sized,
{
    buckets: Vec<*mut [T; BLOCK_SIZE]>,
    len: usize,
}

unsafe impl<T: Send> Send for ArrayVec<T> {}
unsafe impl<T: Send> Sync for ArrayVec<T> {}

impl<T> ArrayVec<T> {
    pub fn new() -> ArrayVec<T> {
        Self {
            buckets: Vec::new(),
            len: 0,
        }
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn get(&self, index: usize) -> Option<&T> {
        if index < self.len {
            let bucket = index / BLOCK_SIZE;
            let offset = index % BLOCK_SIZE;
            unsafe { Some(&(*self.buckets[bucket])[offset]) }
        } else {
            None
        }
    }

    pub fn push(&mut self, val: T) {
        let bucket = self.len / BLOCK_SIZE;
        let offset = self.len % BLOCK_SIZE;
        if self.buckets.len() <= bucket {
            let data: [T; BLOCK_SIZE] = unsafe { mem::uninitialized() };
            self.buckets.push(Box::into_raw(Box::new(data)));
        }
        unsafe {
            ptr::write(&mut (*self.buckets[bucket])[offset], val);
        }
        self.len += 1;
    }

    pub fn iter(&self) -> Iter<T> {
        Iter { v: self, offset: 0 }
    }
}

pub struct Iter<'a, T>
where
    T: 'a,
{
    v: &'a ArrayVec<T>,
    offset: usize,
}

impl<'a, T> Iterator for Iter<'a, T> {
    type Item = &'a T;

    fn next(&mut self) -> Option<Self::Item> {
        let val = self.v.get(self.offset);
        self.offset += 1;
        val
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.v.len(), Some(self.v.len()))
    }
}
