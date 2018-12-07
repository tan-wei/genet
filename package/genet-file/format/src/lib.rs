use genet_sdk::prelude::*;
use serde_derive::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct Header {
    pub layer_id: String,
    pub entries: usize,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Frame {
    pub len: usize,
}

pub struct Entry {
    pub frame: Frame,
    pub data: ByteSlice,
}
