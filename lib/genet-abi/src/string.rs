use super::vec::SafeVec;
use std::{
    ops::{Deref, DerefMut},
    str,
};

#[repr(C)]
#[derive(Debug, Clone)]
pub struct SafeString {
    data: SafeVec<u8>,
}

impl SafeString {
    pub fn new() -> SafeString {
        SafeString {
            data: SafeVec::new(),
        }
    }

    pub fn as_str(&self) -> &str {
        unsafe { str::from_utf8_unchecked(&self.data) }
    }

    pub fn as_str_mut(&mut self) -> &mut str {
        unsafe { str::from_utf8_unchecked_mut(&mut self.data) }
    }
}

impl<'a> From<&'a str> for SafeString {
    fn from(s: &'a str) -> SafeString {
        SafeString {
            data: SafeVec::from(s.as_bytes()),
        }
    }
}

impl Deref for SafeString {
    type Target = str;

    fn deref(&self) -> &str {
        self.as_str()
    }
}

impl DerefMut for SafeString {
    fn deref_mut(&mut self) -> &mut str {
        self.as_str_mut()
    }
}

impl PartialEq for SafeString {
    #[inline]
    fn eq(&self, other: &SafeString) -> bool {
        PartialEq::eq(&self[..], &other[..])
    }

    #[inline]
    fn ne(&self, other: &SafeString) -> bool {
        PartialEq::ne(&self[..], &other[..])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn eq() {
        let s1 = SafeString::from("abc");
        let s2 = SafeString::from("abc");
        let s3 = SafeString::from("abcd");
        assert_eq!(s1, s2);
        assert_eq!(s1 == s3, false);
        assert_eq!(s2 == s3, false);
    }

    #[test]
    fn as_str() {
        let s = SafeString::from("abc");
        assert_eq!(s.as_str(), "abc");
    }
}
