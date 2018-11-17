extern crate genet_sdk;
extern crate serde;

#[macro_use]
extern crate serde_derive;

use genet_sdk::prelude::*;

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
