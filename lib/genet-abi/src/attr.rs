use decoder::{Decoder, Nil};
use env;
use error::Error;
use layer::Layer;
use fixed::Fixed;
use result::Result;
use slice::ByteSlice;
use std::{fmt, mem, ops::Range, slice};
use token::Token;
use variant::Variant;
use vec::SafeVec;

#[repr(C)]
pub struct Attr {
    class: Fixed<AttrClass>,
    abi_unsafe_data: AttrData,
}

impl fmt::Debug for Attr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Attr")
    }
}

struct AttrData {
    range: Range<usize>,
    value: Option<Fixed<Variant>>,
}

impl Attr {
    pub fn new<C: Into<Fixed<AttrClass>>>(class: C, range: Range<usize>) -> Attr {
        Attr {
            class: class.into(),
            abi_unsafe_data: AttrData { range, value: None },
        }
    }

    pub fn with_value<C: Into<Fixed<AttrClass>>, T: Into<Variant>>(
        class: C,
        range: Range<usize>,
        value: T,
    ) -> Attr {
        Attr {
            class: class.into(),
            abi_unsafe_data: AttrData {
                range,
                value: Some(Fixed::new(value.into())),
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

    pub fn try_get(&self, layer: &Layer) -> Result<Variant> {
        self.class.try_get(self, layer)
    }
}

impl Into<Fixed<Attr>> for Attr {
    fn into(self) -> Fixed<Attr> {
        Fixed::new(self)
    }
}

impl Into<Fixed<Attr>> for &'static Attr {
    fn into(self) -> Fixed<Attr> {
        Fixed::from_static(self)
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
    ByteSlice = 7,
}

pub struct AttrBuilder {
    id: Token,
    typ: Token,
    decoder: Box<Decoder>,
}

impl AttrBuilder {
    pub fn new<T: Into<Token>>(id: T) -> AttrBuilder {
        Self {
            id: id.into(),
            typ: Token::null(),
            decoder: Box::new(Nil()),
        }
    }

    pub fn typ<T: Into<Token>>(mut self, typ: T) -> AttrBuilder {
        self.typ = typ.into();
        self
    }

    pub fn decoder<T: Decoder>(mut self, decoder: T) -> AttrBuilder {
        self.decoder = decoder.clone_box();
        self
    }

    pub fn build(self) -> AttrClass {
        AttrClass {
            abi_unsafe_data: Fixed::new(AttrClassData {
                id: self.id,
                typ: self.typ,
                decoder: self.decoder,
            }),
            id: abi_id,
            typ: abi_typ,
            range: abi_range,
            get: abi_get,
        }
    }
}

#[repr(C)]
pub struct AttrClass {
    abi_unsafe_data: Fixed<AttrClassData>,
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

    fn try_get(&self, attr: &Attr, layer: &Layer) -> Result<Variant> {
        let data = layer.data();
        let data = if let Some(data) = data.get(self.range(attr)) {
            data
        } else {
            return Err(Box::new(Error::new("out of bounds")));
        };
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
            ValueType::ByteSlice => {
                let len: u64 = unsafe { mem::transmute_copy(&num) };
                Ok(Variant::Slice(unsafe {
                    ByteSlice::from_raw_parts(buf, len as usize)
                }))
            }
            _ => Ok(Variant::Nil),
        }
    }
}

impl Into<Fixed<AttrClass>> for &'static AttrClass {
    fn into(self) -> Fixed<AttrClass> {
        Fixed::from_static(self)
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
    let slice = unsafe { ByteSlice::from_raw_parts(*data, len as usize) };
    let res;
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
                ValueType::ByteSlice
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
    use attr::{Attr, AttrBuilder};
    use decoder::Decoder;
    use layer::{Layer, LayerBuilder};
    use slice::{ByteSlice, TryGet};
    use std::{
        error,
        io::{Error, ErrorKind, Result},
        str::from_utf8,
    };
    use token::Token;
    use variant::Variant;

    #[test]
    fn nil() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Nil)
            }
        }
        let class = AttrBuilder::new("nil")
            .typ("@nil")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..0);
        assert_eq!(attr.id(), Token::from("nil"));
        assert_eq!(attr.typ(), Token::from("@nil"));
        assert_eq!(attr.range(), 0..0);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::new());
        match attr.try_get(&layer).unwrap() {
            Variant::Nil => (),
            _ => panic!(),
        };
    }

    #[test]
    fn bool() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Bool(data[0] == 1))
            }
        }
        let class = AttrBuilder::new("bool")
            .typ("@bool")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..1);
        assert_eq!(attr.id(), Token::from("bool"));
        assert_eq!(attr.typ(), Token::from("@bool"));
        assert_eq!(attr.range(), 0..1);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&[1][..]));
        match attr.try_get(&layer).unwrap() {
            Variant::Bool(val) => assert!(val),
            _ => panic!(),
        };
    }

    #[test]
    fn u64() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::UInt64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrBuilder::new("u64")
            .typ("@u64")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("u64"));
        assert_eq!(attr.typ(), Token::from("@u64"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"123456789"[..]));
        match attr.try_get(&layer).unwrap() {
            Variant::UInt64(val) => assert_eq!(val, 123_456),
            _ => panic!(),
        };
    }

    #[test]
    fn i64() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Int64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrBuilder::new("i64")
            .typ("@i64")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("i64"));
        assert_eq!(attr.typ(), Token::from("@i64"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"-123456789"[..]));
        match attr.try_get(&layer).unwrap() {
            Variant::Int64(val) => assert_eq!(val, -12345),
            _ => panic!(),
        };
    }

    #[test]
    fn buffer() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Buffer(data.to_vec().into_boxed_slice()))
            }
        }
        let class = AttrBuilder::new("buffer")
            .typ("@buffer")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("buffer"));
        assert_eq!(attr.typ(), Token::from("@buffer"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"123456789"[..]));
        match attr.try_get(&layer).unwrap() {
            Variant::Buffer(val) => assert_eq!(&*val, b"123456"),
            _ => panic!(),
        };
    }

    #[test]
    fn string() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::String(
                    from_utf8(data).unwrap().to_string().into_boxed_str(),
                ))
            }
        }
        let class = AttrBuilder::new("string")
            .typ("@string")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("string"));
        assert_eq!(attr.typ(), Token::from("@string"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"123456789"[..]));
        match attr.try_get(&layer).unwrap() {
            Variant::String(val) => assert_eq!(&*val, "123456"),
            _ => panic!(),
        };
    }

    #[test]
    fn slice() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, data: &ByteSlice) -> Result<Variant> {
                data.try_get(0..3).map(|v| Variant::Slice(v))
            }
        }
        let class = AttrBuilder::new("slice")
            .typ("@slice")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), Token::from("@slice"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"123456789"[..]));
        match attr.try_get(&layer).unwrap() {
            Variant::Slice(val) => assert_eq!(val, ByteSlice::from(&b"123"[..])),
            _ => panic!(),
        };
    }

    #[test]
    fn error() {
        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &ByteSlice) -> Result<Variant> {
                Err(Error::new(ErrorKind::Other, "oh no!"))
            }
        }
        let class = AttrBuilder::new("slice")
            .typ("@slice")
            .decoder(TestDecoder {})
            .build();
        let attr = Attr::new(&class, 0..6);
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), Token::from("@slice"));
        assert_eq!(attr.range(), 0..6);

        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, ByteSlice::from(&b"123456789"[..]));
        match attr.try_get(&layer) {
            Err(err) => assert_eq!(err.description(), "oh no!"),
            _ => panic!(),
        };
    }
}
