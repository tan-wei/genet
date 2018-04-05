use std::collections::HashMap;

pub mod value {
    use std::result;
    use std::ops;
    use std::ptr;

    #[derive(Debug, Clone, PartialEq)]
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

    pub trait Fn {
        fn get(&self, &[u8]) -> Result;
        fn len(&self, &[u8]) -> usize;
        fn clone(&self) -> Box<Fn>;
    }

    pub type Range = ops::Range<usize>;
    pub type Result = result::Result<Value, Error>;

    #[derive(Debug, Clone, PartialEq)]
    pub enum Value {
        Boolean(bool),
        Int64(i64),
        Uint64(u64),
        Float64(f64),
        Str(String),
        Slice((*const u8, usize)),
    }

    pub struct Slice {
        range: Option<Range>,
    }

    impl Slice {
        pub fn new() -> Box<Slice> {
            Box::from(Slice { range: None })
        }

        pub fn with_range(range: Range) -> Box<Slice> {
            Box::from(Slice { range: Some(range) })
        }
    }

    impl Fn for Slice {
        fn get(&self, data: &[u8]) -> Result {
            if let Some(ref range) = self.range {
                if range.start <= range.end && data.len() >= range.end {
                    if data.len() == 0 {
                        return Ok(Value::Slice((ptr::null(), 0)));
                    }
                    let ptr = unsafe { (&data[0] as *const u8).offset(range.start as isize) };
                    Ok(Value::Slice((ptr, range.len())))
                } else {
                    Err(Error::new_out_of_bound())
                }
            } else if data.len() == 0 {
                Ok(Value::Slice((ptr::null(), 0)))
            } else {
                let ptr = &data[0] as *const u8;
                Ok(Value::Slice((ptr, data.len())))
            }
        }

        fn len(&self, data: &[u8]) -> usize {
            if let Some(ref range) = self.range {
                range.len()
            } else {
                data.len()
            }
        }

        fn clone(&self) -> Box<Fn> {
            Box::from(Slice { range: self.range.clone() })
        }
    }

    #[test]
    fn default_slice() {
        let s = Slice::new();
        let buf = "abc".as_bytes();
        assert_eq!(s.len("".as_bytes()), 0);
        assert_eq!(s.len(buf), buf.len());
        assert_eq!(
            s.get("".as_bytes()).unwrap(),
            Value::Slice((ptr::null(), 0))
        );
        assert_eq!(
            s.get(buf).unwrap(),
            Value::Slice((&buf[0] as *const u8, buf.len()))
        );
    }

    #[test]
    fn range_slice() {
        let s = Slice::with_range(1..3);
        let buf = "abc".as_bytes();
        assert_eq!(s.len("".as_bytes()), 2);
        assert_eq!(s.len(buf), 2);
        assert_eq!(
            s.get("".as_bytes()).err().unwrap(),
            Error::new_out_of_bound()
        );
        assert_eq!(s.get(buf).unwrap(), Value::Slice((&buf[1] as *const u8, 2)));
    }
}

pub struct Field {
    id: u32,
    name: String,
    typ: String,
    val: Box<value::Fn>,
}

impl Clone for Field {
    fn clone(&self) -> Self {
        Field {
            id: self.id,
            name: self.name.clone(),
            typ: self.typ.clone(),
            val: self.val.clone()
        }
    }
}

impl Field {
    pub fn new(id: u32, name: &str, typ: &str, val: Box<value::Fn>) -> Field {
        Field {
            id,
            name: String::from(name),
            typ: String::from(typ),
            val,
        }
    }

    pub fn id(&self) -> u32 {
        self.id
    }

    pub fn name(&self) -> &str {
        self.name.as_str()
    }

    pub fn typ(&self) -> &str {
        self.typ.as_str()
    }

    pub fn apply(&self, data: &[u8]) -> Box<BoundValue> {
        let len = self.val.len(data);
        let val = self.val.get(data);
        Box::new(BoundValue { len, val })
    }
}

#[derive(Debug)]
pub struct BoundValue {
    pub len: usize,
    pub val: value::Result,
}

#[derive(Debug)]
pub struct BoundField {
    id: u32,
    offset: usize,
}

impl BoundField {
    pub fn new(id: u32, offset: usize) -> BoundField {
        BoundField { id, offset }
    }

    pub fn id(&self) -> u32 {
        self.id
    }

    pub fn offset(&self) -> usize {
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

    pub fn register(&mut self, name: &str, typ: &str, val: Box<value::Fn>) -> Field {
        let id = self.map.len() as u32;
        self.map
            .entry(String::from(name))
            .or_insert_with(|| Field::new(id, name, typ, val))
            .clone()
    }

    pub fn get(&mut self, id: &str) -> Option<Field> {
        self.map.get(&String::from(id)).map(|f| f.clone())
    }
}
