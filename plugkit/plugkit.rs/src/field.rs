use std::collections::HashMap;
use std::ops::Range;
use std::result;

#[derive(Debug, Clone)]
pub struct Error {
    typ: String,
    msg: String,
}

impl Error {
    pub fn new(typ: &str, msg: &str) -> Error {
        Error {
            typ: String::from(typ),
            msg: String::from(msg),
        }
    }

    pub fn new_out_of_bound() -> Error {
        Error::new("!out-of-bounds", "")
    }

    pub fn typ(&self) -> &str {
        self.typ.as_str()
    }

    pub fn msg(&self) -> &str {
        self.msg.as_str()
    }
}

type Result = result::Result<Value, Error>;

#[derive(Debug, Clone)]
pub enum Value {
    Boolean(bool),
    Int64(i64),
    Uint64(u64),
    Float64(f64),
    Str(String),
    Slice(Range<u32>),
}

pub trait ValueFn {
    fn get(&self, &Field, &[u8]) -> Result;
    fn len(&self, &Field, &[u8]) -> u32;
}

pub struct Field {
    id: String,
    typ: String,
    val: Box<ValueFn>,
}

impl Field {
    pub fn new(id: &str, typ: &str, val: Box<ValueFn>) -> Field {
        Field {
            id: String::from(id),
            typ: String::from(typ),
            val,
        }
    }

    pub fn id(&self) -> &str {
        self.id.as_str()
    }

    pub fn typ(&self) -> &str {
        self.typ.as_str()
    }

    pub fn apply(&self, data: &[u8]) -> Box<BoundValue> {
        let len = self.val.len(self, data);
        let val = self.val.get(self, data);
        Box::new(BoundValue { len, val })
    }
}

#[derive(Debug)]
pub struct BoundValue {
    pub len: u32,
    pub val: Result,
}

#[derive(Debug)]
pub struct BoundField {
    id: u32,
    offset: u32,
}

impl BoundField {
    pub fn new(id: u32, offset: u32) -> BoundField {
        BoundField { id, offset }
    }

    pub fn id(&self) -> u32 {
        self.id
    }

    pub fn offset(&self) -> u32 {
        self.offset
    }
}

pub struct Registry {
    map: HashMap<String, Field>,
}

impl Registry {
    pub fn new() -> Registry {
        Registry {
            map: HashMap::new(),
        }
    }

    pub fn register(&mut self, id: &str, typ: &str, val: Box<ValueFn>) -> &mut Field {
        self.map
            .entry(String::from(id))
            .or_insert_with(|| Field::new(id, typ, val))
    }

    pub fn get(&mut self, id: &str) -> Option<&Field> {
        self.map.get(&String::from(id)).map(|attr| attr.clone())
    }
}
