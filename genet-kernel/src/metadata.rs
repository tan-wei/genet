use evmap::ShallowCopy;
use serde::ser::{Serialize, SerializeMap, Serializer};

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Metadata {
    Linkage(Vec<u32>),
    Error(String),
}

impl ShallowCopy for Metadata {
    unsafe fn shallow_copy(&mut self) -> Self {
        self.clone()
    }
}

impl Serialize for Metadata {
    fn serialize<S>(&self, serializer: S) -> ::std::result::Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match self {
            Metadata::Linkage(indices) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "linkage")?;
                s.serialize_entry("indices", &indices)?;
                s.end()
            }
            Metadata::Error(message) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "error")?;
                s.serialize_entry("message", &message)?;
                s.end()
            }
        }
    }
}
