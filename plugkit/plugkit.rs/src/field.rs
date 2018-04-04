use std::collections::HashMap;

#[derive(Debug, Clone)]
pub enum Value {
    Nil,
    Boolean(bool),
    Int64(i64),
    Uint64(u64),
    Float64(f64),
    Str(String),
    Slice((u32, u32)),
}

pub enum ValueFn {
    Const(Value),
    Func(fn(&Field, &[u8]) -> Option<Value>),
    Script((String, fn(&Field, &str, &[u8]) -> Option<Value>)),
}

pub enum LengthFn {
    Const(u32),
    Func(fn(&Field, &[u8]) -> u32),
    Script((String, fn(&Field, &str, &[u8]) -> u32)),
}

pub struct Field {
    id: String,
    typ: String,
    len: LengthFn,
    val: ValueFn,
}

impl Field {
    pub fn new(id: &str, typ: &str, len: LengthFn, val: ValueFn) -> Field {
        Field {
            id: String::from(id),
            typ: String::from(typ),
            len,
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
        let len = match self.len {
            LengthFn::Const(val) => val,
            LengthFn::Func(func) => func(self, data),
            LengthFn::Script((ref code, func)) => func(self, code.as_str(), data),
        };
        let val = match self.val {
            ValueFn::Const(ref val) => Some(val.clone()),
            ValueFn::Func(func) => func(self, data),
            ValueFn::Script((ref code, func)) => func(self, code.as_str(), data),
        };
        Box::new(BoundValue { len, val })
    }
}

#[derive(Debug)]
pub struct BoundValue {
    pub len: u32,
    pub val: Option<Value>,
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

    pub fn register(
        &mut self,
        id: &str,
        typ: &str,
        len: LengthFn,
        val: ValueFn,
    ) -> &mut Field {
        self.map
            .entry(String::from(id))
            .or_insert_with(|| Field::new(id, typ, len, val))
    }

    pub fn get(&mut self, id: &str) -> Option<&Field> {
        self.map.get(&String::from(id)).map(|attr| attr.clone())
    }
}
