use std::{iter::Skip, mem, ops::Range, ptr};

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
            let mut data: [T; BLOCK_SIZE] = unsafe { mem::uninitialized() };
            self.buckets.push(Box::into_raw(Box::new(data)));
        }
        unsafe {
            ptr::write(&mut (*self.buckets[bucket])[offset], val);
        }
        self.len += 1;
    }

    pub fn append(&mut self, vec: Vec<T>) {
        for val in vec.into_iter() {
            self.push(val);
        }
    }

    pub fn iter(&self) -> Iter<T> {
        Iter { v: self, offset: 0 }
    }

    pub fn slice(&self, range: Range<usize>) -> ::std::vec::IntoIter<&[T]> {
        let mut v = Vec::new();
        if self.buckets.is_empty() {
            return v.into_iter();
        }
        let start = if range.start < self.len {
            range.start
        } else {
            self.len
        };
        let end = if range.end < self.len {
            range.end
        } else {
            self.len
        };
        let start_bucket = start / BLOCK_SIZE;
        let start_offset = start % BLOCK_SIZE;
        let end_bucket = end / BLOCK_SIZE;
        let end_offset = end % BLOCK_SIZE;

        if start_bucket == end_bucket {
            unsafe { v.push(&(*self.buckets[start_bucket])[start_offset..end_bucket]) };
        } else {
            for i in (start_bucket..end_bucket) {
                if i == start_bucket {
                    unsafe { v.push(&(*self.buckets[i])[start_offset..]) };
                } else if i == end_bucket - 1 {
                    unsafe { v.push(&(*self.buckets[i])[..end_offset]) };
                } else {
                    unsafe { v.push(&(*self.buckets[i])[..]) };
                }
            }
        }
        v.into_iter()
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
