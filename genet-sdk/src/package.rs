use crate::decoder::DecoderBox;
use serde::ser;
use serde_derive::{Deserialize, Serialize};

pub struct Copyable<T> {
    data: T,
}

impl<T> Copyable<T> {
    unsafe fn new(data: T) -> Self {
        Self { data }
    }
}

impl<T> ser::Serialize for Copyable<T> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: ser::Serializer,
    {
        serializer.serialize_i32(0)
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
        //decoder: Copyable<DecoderBox>,
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
