use decoder::Decoder;
use env;
use error::Error;
use layer::Layer;
use ptr::Ptr;
use result::Result;
use std::fmt;
use std::mem;
use std::ops::Range;
use std::slice;
use token::Token;
use variant::Variant;
use vec::SafeVec;

#[repr(C)]
pub struct Attr {
    class: Ptr<AttrClass>,
    abi_unsafe_data: AttrData,
}

impl fmt::Debug for Attr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Attr")
    }
}

struct AttrData {
    range: Range<usize>,
    value: Option<Ptr<Variant>>,
}

impl Attr {
    pub fn new(class: &Ptr<AttrClass>, range: Range<usize>) -> Attr {
        Attr {
            class: class.clone(),
            abi_unsafe_data: AttrData { range, value: None },
        }
    }

    pub fn with_value(class: Ptr<AttrClass>, range: Range<usize>, value: Variant) -> Attr {
        Attr {
            class: class.clone(),
            abi_unsafe_data: AttrData {
                range,
                value: Some(Ptr::new(value)),
            },
        }
    }

    pub fn id(&self) -> Token {
        self.class.id()
    }

    pub fn typ(&self) -> Token {
        self.class.typ()
    }

    pub fn range(&self) -> Range<usize> {
        self.class.range(self)
    }

    pub fn data(&self, layer: &Layer) -> Result<Variant> {
        self.class.data(self, layer)
    }
}

#[repr(i8)]
enum ValueType {
    Error = -1,
    Nil = 0,
    Bool = 1,
    Int64 = 2,
    UInt64 = 3,
    Float64 = 4,
    String = 5,
    Buffer = 6,
    Slice = 7,
}

#[repr(u64)]
#[derive(PartialEq, PartialOrd)]
#[allow(dead_code)]
enum Revision {
    Id = 0,
    Typ = 1,
    Range = 2,
}

#[repr(C)]
pub struct AttrClass {
    rev: Revision,
    abi_unsafe_data: Ptr<AttrClassData>,
    id: extern "C" fn(class: *const AttrClass) -> Token,
    typ: extern "C" fn(class: *const AttrClass) -> Token,
    range: extern "C" fn(*const Attr, *mut u64, *mut u64),
    get: extern "C" fn(*const Attr, *mut *const u8, u64, *mut i64, *mut Error) -> ValueType,
}

struct AttrClassData {
    id: Token,
    typ: Token,
    decoder: Box<Decoder>,
}

impl AttrClass {
    pub fn new<T: Decoder>(id: Token, typ: Token, decoder: T) -> Ptr<AttrClass> {
        Ptr::new(AttrClass {
            rev: Revision::Range,
            abi_unsafe_data: Ptr::new(AttrClassData {
                id,
                typ,
                decoder: decoder.clone_box(),
            }),
            id: abi_id,
            typ: abi_typ,
            range: abi_range,
            get: abi_get,
        })
    }

    fn id(&self) -> Token {
        (self.id)(self)
    }

    fn typ(&self) -> Token {
        (self.typ)(self)
    }

    fn range(&self, attr: &Attr) -> Range<usize> {
        let mut start;
        let mut end;
        unsafe {
            start = mem::uninitialized();
            end = mem::uninitialized();
        }
        (self.range)(attr, &mut start, &mut end);
        start as usize..end as usize
    }

    fn data(&self, attr: &Attr, layer: &Layer) -> Result<Variant> {
        let data = &layer.data()[self.range(attr)];
        let mut buf: *const u8 = data.as_ptr();
        let mut num = 0;
        let mut err = Error::new("");
        let typ = (self.get)(attr, &mut buf, data.len() as u64, &mut num, &mut err);
        match typ {
            ValueType::Error => Err(Box::new(err)),
            ValueType::Bool => Ok(Variant::Bool(num == 1)),
            ValueType::Int64 => Ok(Variant::Int64(num)),
            ValueType::UInt64 => Ok(Variant::UInt64(unsafe { mem::transmute_copy(&num) })),
            ValueType::Float64 => Ok(Variant::Float64(unsafe { mem::transmute_copy(&num) })),
            ValueType::Buffer => unsafe {
                let len: u64 = mem::transmute_copy(&num);
                let b = Box::from(slice::from_raw_parts(buf, len as usize));
                env::dealloc(buf as *mut u8);
                Ok(Variant::Buffer(b))
            },
            ValueType::String => unsafe {
                let len: u64 = mem::transmute_copy(&num);
                let s =
                    String::from_utf8_unchecked(slice::from_raw_parts(buf, len as usize).to_vec());
                env::dealloc(buf as *mut u8);
                Ok(Variant::String(s.into_boxed_str()))
            },
            ValueType::Slice => {
                let len: u64 = unsafe { mem::transmute_copy(&num) };
                Ok(Variant::Slice(unsafe {
                    slice::from_raw_parts(buf, len as usize)
                }))
            }
            _ => Ok(Variant::Nil),
        }
    }
}

extern "C" fn abi_range(attr: *const Attr, start: *mut u64, end: *mut u64) {
    unsafe {
        let range = &(*attr).abi_unsafe_data.range;
        *start = range.start as u64;
        *end = range.end as u64;
    }
}

extern "C" fn abi_id(class: *const AttrClass) -> Token {
    unsafe { (*class).abi_unsafe_data.id }
}

extern "C" fn abi_typ(class: *const AttrClass) -> Token {
    unsafe { (*class).abi_unsafe_data.typ }
}

extern "C" fn abi_get(
    attr: *const Attr,
    data: *mut *const u8,
    len: u64,
    num: *mut i64,
    err: *mut Error,
) -> ValueType {
    let value = unsafe { &(*attr).abi_unsafe_data.value };
    let decoder = unsafe { &(*attr).class.abi_unsafe_data.decoder };
    let slice = unsafe { slice::from_raw_parts(*data, len as usize) };
    let mut res = Ok(Variant::Nil);
    let result = if let Some(val) = value {
        Ok(val.as_ref())
    } else {
        res = decoder.decode(&slice);
        res.as_ref()
    };
    match result {
        Ok(v) => match v {
            Variant::Bool(val) => {
                unsafe { *num = if *val { 1 } else { 0 } };
                ValueType::Bool
            }
            Variant::Int64(val) => {
                unsafe { *num = *val };
                ValueType::Int64
            }
            Variant::UInt64(val) => {
                unsafe { *(num as *mut u64) = *val };
                ValueType::UInt64
            }
            Variant::Float64(val) => {
                unsafe { *(num as *mut f64) = *val };
                ValueType::Float64
            }
            Variant::Buffer(val) => {
                unsafe {
                    let (buf, len) = SafeVec::into_raw(SafeVec::from(val));
                    *data = buf;
                    *(num as *mut u64) = len as u64;
                };
                ValueType::Buffer
            }
            Variant::String(val) => {
                unsafe {
                    let (buf, len) = SafeVec::into_raw(SafeVec::from(val.as_bytes()));
                    *data = buf;
                    *(num as *mut u64) = len as u64;
                };
                ValueType::String
            }
            Variant::Slice(val) => {
                unsafe {
                    *data = val.as_ptr();
                    *(num as *mut u64) = val.len() as u64;
                };
                ValueType::Slice
            }
            _ => ValueType::Nil,
        },
        Err(e) => {
            unsafe { *err = Error::new(::std::error::Error::description(&*e)) }
            ValueType::Error
        }
    }
}

#[cfg(test)]
mod tests {
    use attr::{Attr, AttrClass};
    use decoder::Decoder;
    use env;
    use layer::{Layer, LayerClass};
    use slice::Slice;
    use std::error;
    use std::io::{Error, ErrorKind, Result};
    use std::str::from_utf8;
    use variant::Variant;

    #[test]
    fn nil() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &Slice) -> Result<Variant> {
                Ok(Variant::Nil)
            }
        }
        let class = AttrClass::new(env::token("nil"), env::token("@nil"), TestDecoder {});
        let attr = Attr::new(&class, 0..0);
        assert_eq!(attr.id(), env::token("nil"));
        assert_eq!(attr.typ(), env::token("@nil"));
        assert_eq!(attr.range(), 0..0);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, &[]);
        match attr.data(&layer).unwrap() {
            Variant::Nil => (),
            _ => panic!(),
        };
    }

    #[test]
    fn bool() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::Bool(data[0] == 1))
            }
        }
        let class = AttrClass::new(env::token("bool"), env::token("@bool"), TestDecoder {});
        let attr = Attr::new(&class, 0..1);
        assert_eq!(attr.id(), env::token("bool"));
        assert_eq!(attr.typ(), env::token("@bool"));
        assert_eq!(attr.range(), 0..1);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, &[1]);
        match attr.data(&layer).unwrap() {
            Variant::Bool(val) => assert!(val),
            _ => panic!(),
        };
    }

    #[test]
    fn u64() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::UInt64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrClass::new(env::token("u64"), env::token("@u64"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("u64"));
        assert_eq!(attr.typ(), env::token("@u64"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"123456789");
        match attr.data(&layer).unwrap() {
            Variant::UInt64(val) => assert_eq!(val, 123456),
            _ => panic!(),
        };
    }

    #[test]
    fn i64() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::Int64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrClass::new(env::token("i64"), env::token("@i64"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("i64"));
        assert_eq!(attr.typ(), env::token("@i64"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"-123456789");
        match attr.data(&layer).unwrap() {
            Variant::Int64(val) => assert_eq!(val, -12345),
            _ => panic!(),
        };
    }

    #[test]
    fn buffer() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::Buffer(data.to_vec().into_boxed_slice()))
            }
        }
        let class = AttrClass::new(env::token("buffer"), env::token("@buffer"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("buffer"));
        assert_eq!(attr.typ(), env::token("@buffer"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"123456789");
        match attr.data(&layer).unwrap() {
            Variant::Buffer(val) => assert_eq!(&*val, b"123456"),
            _ => panic!(),
        };
    }

    #[test]
    fn string() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::String(
                    from_utf8(data).unwrap().to_string().into_boxed_str(),
                ))
            }
        }
        let class = AttrClass::new(env::token("string"), env::token("@string"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("string"));
        assert_eq!(attr.typ(), env::token("@string"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"123456789");
        match attr.data(&layer).unwrap() {
            Variant::String(val) => assert_eq!(&*val, "123456"),
            _ => panic!(),
        };
    }

    #[test]
    fn slice() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &Slice) -> Result<Variant> {
                Ok(Variant::Slice(&data[0..3]))
            }
        }
        let class = AttrClass::new(env::token("slice"), env::token("@slice"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("slice"));
        assert_eq!(attr.typ(), env::token("@slice"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"123456789");
        match attr.data(&layer).unwrap() {
            Variant::Slice(val) => assert_eq!(val, b"123"),
            _ => panic!(),
        };
    }

    #[test]
    fn error() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &Slice) -> Result<Variant> {
                Err(From::from(Error::new(ErrorKind::Other, "oh no!")))
            }
        }
        let class = AttrClass::new(env::token("slice"), env::token("@slice"), TestDecoder {});
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), env::token("slice"));
        assert_eq!(attr.typ(), env::token("@slice"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerClass::new(0);
        let layer = Layer::new(&class, b"123456789");
        match attr.data(&layer) {
            Err(err) => assert_eq!(err.description(), "oh no!"),
            _ => panic!(),
        };
    }
}
