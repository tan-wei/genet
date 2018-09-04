extern crate genet_sdk;
extern crate serde;

#[macro_use]
extern crate serde_derive;

use genet_sdk::{prelude::*, variant::Variant};

#[derive(Debug, Serialize, Deserialize)]
pub struct Header {
    pub tokens: Vec<String>,
    pub attrs: Vec<AttrClass>,
    pub entries: usize,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Value {
    Nil,
    Bool(bool),
    Int64(i64),
    UInt64(u64),
    Float64(f64),
    String(Box<str>),
    Buffer(Box<[u8]>),
}

impl From<Variant> for Value {
    fn from(v: Variant) -> Value {
        match v {
            Variant::Bool(x) => Value::Bool(x),
            Variant::Int64(x) => Value::Int64(x),
            Variant::UInt64(x) => Value::UInt64(x),
            Variant::Float64(x) => Value::Float64(x),
            Variant::String(x) => Value::String(x),
            Variant::Buffer(x) => Value::Buffer(x),
            _ => Value::Nil,
        }
    }
}

impl Into<Variant> for Value {
    fn into(self) -> Variant {
        match self {
            Value::Bool(x) => Variant::Bool(x),
            Value::Int64(x) => Variant::Int64(x),
            Value::UInt64(x) => Variant::UInt64(x),
            Value::Float64(x) => Variant::Float64(x),
            Value::String(x) => Variant::String(x),
            Value::Buffer(x) => Variant::Buffer(x),
            _ => Variant::Nil,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct AttrClass {
    pub id: usize,
    pub typ: usize,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Attr {
    pub index: usize,
    pub value: Value,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Frame {
    pub id: usize,
    pub len: usize,
    pub attrs: Vec<Attr>,
}

pub struct Entry {
    pub frame: Frame,
    pub data: ByteSlice,
}
