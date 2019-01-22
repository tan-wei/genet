use serde::{
    de::{self, Deserialize, Visitor},
    ser,
};
use std::{
    fmt, mem,
    ops::{Deref, DerefMut},
    ptr,
};

pub unsafe trait Codable: Copy {}

#[derive(Clone)]
pub struct CodedData<T: Codable> {
    data: T,
}

impl<T: Codable> Deref for CodedData<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.data
    }
}

impl<T: Codable> DerefMut for CodedData<T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut self.data
    }
}

impl<T: Codable> CodedData<T> {
    pub fn new(data: T) -> Self {
        Self { data }
    }
}

impl<T: Codable> ser::Serialize for CodedData<T> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: ser::Serializer,
        T: Sized,
    {
        let mut buf = vec![0; mem::size_of::<T>()];
        unsafe { ptr::write(buf.as_mut_ptr() as *mut T, self.data) };
        serializer.serialize_bytes(&buf)
    }
}

struct BufVisitor;

impl<'de> Visitor<'de> for BufVisitor {
    type Value = &'de [u8];

    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        formatter.write_str("a byte slice")
    }

    fn visit_borrowed_bytes<E>(self, value: &'de [u8]) -> Result<Self::Value, E>
    where
        E: de::Error,
    {
        Ok(value)
    }
}

impl<'de, T: Codable> Deserialize<'de> for CodedData<T> {
    fn deserialize<D>(deserializer: D) -> Result<CodedData<T>, D::Error>
    where
        D: de::Deserializer<'de>,
    {
        let buf = deserializer.deserialize_bytes(BufVisitor)?;
        unsafe { Ok(CodedData::new(ptr::read(mem::transmute(buf.as_ptr())))) }
    }
}
