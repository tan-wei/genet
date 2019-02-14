use std::{
    fmt, mem,
    ptr::{self, NonNull},
    slice, str,
};

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Token(NonNull<Header>);

unsafe impl Send for Token {}
unsafe impl Sync for Token {}

impl fmt::Debug for Token {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{} ({})", self.as_str(), self.index())
    }
}

impl PartialEq for Token {
    fn eq(&self, other: &Token) -> bool {
        self.0 == other.0 || self.index() == other.index()
    }
}

impl Eq for Token {}

impl Token {
    pub(crate) fn new(index: u32, s: &str) -> Self {
        let ptr;

        if s.is_empty() {
            let header = Header { index, strlen: 0 };
            ptr = Box::into_raw(Box::new(header));
        } else {
            let header = Header {
                index: 0,
                strlen: s.len() as u32,
            };
            let header_size = mem::size_of::<Header>();
            let size = 1 + (s.len() + header_size - 1) / header_size;
            let mut v = vec![header; size];
            let bytes = s.as_bytes();
            unsafe {
                ptr::copy_nonoverlapping(bytes.as_ptr(), v.as_mut_ptr() as *mut u8, bytes.len());
            }
            ptr = v.as_mut_ptr();
            mem::forget(v);
        }
        unsafe { Self(NonNull::new_unchecked(ptr)) }
    }

    pub fn as_str(&self) -> &str {
        let header = self.header();
        if header.strlen == 0 {
            ""
        } else {
            unsafe {
                let data = self.0.as_ptr().offset(1) as *const u8;
                str::from_utf8_unchecked(slice::from_raw_parts(data, header.strlen as usize))
            }
        }
    }

    pub fn index(&self) -> u32 {
        self.header().index
    }

    fn header(&self) -> &Header {
        unsafe { self.0.as_ref() }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
struct Header {
    pub index: u32,
    pub strlen: u32,
}
