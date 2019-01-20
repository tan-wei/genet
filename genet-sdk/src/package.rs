use crate::decoder::DecoderBox;
use serde::{
    de::{self, Deserialize, Visitor},
    ser,
};
use serde_derive::{Deserialize, Serialize};
use std::{fmt, mem, ptr};

pub struct ByteData<T: Clone> {
    data: T,
}

impl<T: Clone> ByteData<T> {
    unsafe fn new(data: T) -> Self {
        Self { data }
    }

    pub fn unwrap(self) -> T {
        self.data
    }
}

impl<T: Clone> ser::Serialize for ByteData<T> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: ser::Serializer,
        T: Sized,
    {
        let mut buf = vec![0; mem::size_of::<T>()];
        unsafe { ptr::write(buf.as_mut_ptr() as *mut T, self.data.clone()) };
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

impl<'de, T: Clone> Deserialize<'de> for ByteData<T> {
    fn deserialize<D>(deserializer: D) -> Result<ByteData<T>, D::Error>
    where
        D: de::Deserializer<'de>,
    {
        let buf = deserializer.deserialize_bytes(BufVisitor)?;
        unsafe { Ok(ByteData::new(ptr::read(mem::transmute(buf.as_ptr())))) }
    }
}

#[derive(Deserialize, Serialize)]
pub struct Package {
    id: String,
    name: String,
    components: Vec<Component>,
}

#[derive(Deserialize, Serialize)]
pub enum Component {
    Decoder {
        id: String,
        trigger_after: Vec<String>,
        decoder: ByteData<DecoderBox>,
    },
    Reader {
        id: String,
        filters: Vec<FileType>,
    },
    Writer {
        id: String,
        filters: Vec<FileType>,
    },
}

#[derive(Serialize, Deserialize)]
pub struct FileType {
    name: String,
    extensions: Vec<String>,
}
