use cast::{Cast, Typed};
use env;
use error::Error;
use fixed::Fixed;
use layer::Layer;
use metadata::Metadata;
use result::Result;
use slice::{ByteSlice, TryGet};
use std::{fmt, io, mem, ops::Range, slice};
use token::Token;
use variant::Variant;
use vec::SafeVec;

/// An attribute object.
#[repr(C)]
pub struct Attr<'a> {
    class: &'a AttrClass,
    range: Range<usize>,
    data: Option<ByteSlice>,
}

impl<'a> fmt::Debug for Attr<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Attr {:?}", self.id())
    }
}

impl<'a> Attr<'a> {
    pub fn new(class: &'a AttrClass, range: Range<usize>, data: Option<ByteSlice>) -> Attr<'a> {
        Attr { class, range, data }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.class.id()
    }

    /// Returns the type of self.
    pub fn typ(&self) -> Token {
        self.class.typ()
    }

    /// Returns the byte range of self.
    pub fn range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    /// Returns the bit range of self.
    pub fn bit_range(&self) -> Range<usize> {
        self.range.clone()
    }

    /// Returns the attribute value.
    pub fn try_get(&self) -> Result<Variant> {
        self.class.try_get_attr(self)
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

#[derive(Clone)]
struct Const<T>(pub T);

impl<T: Into<Variant> + Clone> Typed for Const<T> {
    type Output = T;

    fn cast(&self, _attr: &Attr, _data: &ByteSlice) -> io::Result<T> {
        Ok(self.0.clone())
    }
}

/// A builder object for AttrClass.
pub struct AttrClassBuilder {
    id: Token,
    typ: Token,
    meta: Metadata,
    range: Range<usize>,
    children: Vec<Fixed<AttrClass>>,
    cast: Box<Cast>,
}

impl AttrClassBuilder {
    /// Sets a type of AttrClass.
    pub fn typ<T: Into<Token>>(mut self, typ: T) -> AttrClassBuilder {
        self.typ = typ.into();
        self
    }

    /// Sets a name of AttrClass.
    pub fn name(mut self, name: &'static str) -> AttrClassBuilder {
        self.meta.set_name(name);
        self
    }

    /// Sets a description of AttrClass.
    pub fn description(mut self, desc: &'static str) -> AttrClassBuilder {
        self.meta.set_description(desc);
        self
    }

    /// Sets a byte range of Attr.
    pub fn range(mut self, range: Range<usize>) -> AttrClassBuilder {
        self.range = (range.start * 8)..(range.end * 8);
        self
    }

    /// Sets a bit range of Attr.
    pub fn bit_range(mut self, byte_offset: usize, range: Range<usize>) -> AttrClassBuilder {
        self.range = (range.start + byte_offset * 8)..(range.end + byte_offset * 8);
        self
    }

    /// Sets a cast of AttrClass.
    pub fn cast<T: Cast>(mut self, cast: &T) -> AttrClassBuilder {
        self.cast = cast.clone_box();
        self
    }

    pub fn child<C: Into<Fixed<AttrClass>>>(mut self, class: C) -> AttrClassBuilder {
        self.children.push(class.into());
        self
    }

    /// Sets a constant value of AttrClass.
    pub fn value<T: 'static + Into<Variant> + Send + Sync + Clone>(
        mut self,
        value: T,
    ) -> AttrClassBuilder {
        self.cast = Box::new(Const(value));
        self
    }

    /// Builds a new AttrClass.
    pub fn build(self) -> AttrClass {
        AttrClass {
            get_id: abi_id,
            get_typ: abi_typ,
            get: abi_get,
            id: self.id,
            typ: self.typ,
            meta: self.meta,
            range: self.range,
            children: self.children,
            cast: self.cast,
        }
    }
}

/// An attribute class.
#[repr(C)]
pub struct AttrClass {
    get_id: extern "C" fn(class: *const AttrClass) -> Token,
    get_typ: extern "C" fn(class: *const AttrClass) -> Token,
    get: extern "C" fn(*const Attr, *mut *const u8, u64, *mut i64, *mut Error) -> ValueType,
    id: Token,
    typ: Token,
    meta: Metadata,
    range: Range<usize>,
    children: Vec<Fixed<AttrClass>>,
    cast: Box<Cast>,
}

impl AttrClass {
    /// Creates a new builder object for AttrClass.
    pub fn builder<T: Into<Token>>(id: T) -> AttrClassBuilder {
        AttrClassBuilder {
            id: id.into(),
            typ: Token::null(),
            meta: Metadata::new(),
            range: 0..0,
            children: Vec::new(),
            cast: Box::new(Const(true)),
        }
    }

    pub fn id(&self) -> Token {
        (self.get_id)(self)
    }

    pub fn typ(&self) -> Token {
        (self.get_typ)(self)
    }

    pub fn range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    pub fn bit_range(&self) -> Range<usize> {
        self.range.clone()
    }

    pub fn expand<'a>(
        attr: &'a AttrClass,
        data: &ByteSlice,
        bit_range: Option<Range<usize>>,
    ) -> Vec<Attr<'a>> {
        let range = if let Some(range) = bit_range {
            range
        } else {
            attr.bit_range()
        };
        let byte_range = (range.start / 8)..((range.end + 7) / 8);
        let mut attrs = vec![Attr::new(
            attr,
            range.clone(),
            data.try_get(byte_range.clone()).ok(),
        )];
        for child in &attr.children {
            let offset = byte_range.start;
            let range =
                (child.bit_range().start + offset * 8)..(child.bit_range().end + offset * 8);
            let byte_range = (child.range().start + offset)..(child.range().end + offset);
            attrs.push(Attr::new(&child, range, data.try_get(byte_range).ok()))
        }
        attrs
    }

    pub fn try_get(&self, layer: &Layer) -> Result<Variant> {
        self.try_get_range(layer, self.range())
    }

    pub fn try_get_range(&self, layer: &Layer, range: Range<usize>) -> Result<Variant> {
        let data = layer.data().try_get(range).ok();
        let attr = Attr::new(self, self.bit_range(), data);
        self.try_get_attr(&attr)
    }

    fn try_get_attr(&self, attr: &Attr) -> Result<Variant> {
        let data = if let Some(data) = attr.data {
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

extern "C" fn abi_id(class: *const AttrClass) -> Token {
    unsafe { (*class).id }
}

extern "C" fn abi_typ(class: *const AttrClass) -> Token {
    unsafe { (*class).typ }
}

extern "C" fn abi_get(
    attr: *const Attr,
    data: *mut *const u8,
    len: u64,
    num: *mut i64,
    err: *mut Error,
) -> ValueType {
    let attr = unsafe { &(*attr) };
    let cast = &attr.class.cast;
    let slice = unsafe { ByteSlice::from_raw_parts(*data, len as usize) };

    match cast.cast(attr, &slice) {
        Ok(v) => match v {
            Variant::Bool(val) => {
                unsafe { *num = if val { 1 } else { 0 } };
                ValueType::Bool
            }
            Variant::Int64(val) => {
                unsafe { *num = val };
                ValueType::Int64
            }
            Variant::UInt64(val) => {
                unsafe { *(num as *mut u64) = val };
                ValueType::UInt64
            }
            Variant::Float64(val) => {
                unsafe { *(num as *mut f64) = val };
                ValueType::Float64
            }
            Variant::Buffer(val) => {
                unsafe {
                    let (buf, len) = SafeVec::into_raw(SafeVec::from(&val));
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
            unsafe { *err = Error::new(::std::error::Error::description(&e)) }
            ValueType::Error
        }
    }
}

#[cfg(test)]
mod tests {
    use attr::{Attr, AttrClass};
    use cast::Cast;
    use slice::{ByteSlice, TryGet};
    use std::{
        io::{Error, ErrorKind, Result},
        str::from_utf8,
    };
    use token::Token;
    use variant::Variant;

    #[test]
    fn bool() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Bool(data[0] == 1))
            }
        }
        let class = AttrClass::builder("bool")
            .typ("@bool")
            .range(0..1)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..8, Some(ByteSlice::from(&[1][..])));
        assert_eq!(attr.id(), Token::from("bool"));
        assert_eq!(attr.typ(), Token::from("@bool"));
        assert_eq!(attr.range(), 0..1);

        match attr.try_get().unwrap() {
            Variant::Bool(val) => assert!(val),
            _ => panic!(),
        };
    }

    #[test]
    fn u64() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::UInt64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrClass::builder("u64")
            .typ("@u64")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"123456789"[..])));
        assert_eq!(attr.id(), Token::from("u64"));
        assert_eq!(attr.typ(), Token::from("@u64"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get().unwrap() {
            Variant::UInt64(val) => assert_eq!(val, 123_456_789),
            _ => panic!(),
        };
    }

    #[test]
    fn i64() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Int64(from_utf8(data).unwrap().parse().unwrap()))
            }
        }
        let class = AttrClass::builder("i64")
            .typ("@i64")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"-123456789"[..])));
        assert_eq!(attr.id(), Token::from("i64"));
        assert_eq!(attr.typ(), Token::from("@i64"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get().unwrap() {
            Variant::Int64(val) => assert_eq!(val, -123456789),
            _ => panic!(),
        };
    }

    #[test]
    fn buffer() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Buffer(data.to_vec().into_boxed_slice()))
            }
        }
        let class = AttrClass::builder("buffer")
            .typ("@buffer")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"123456789"[..])));
        assert_eq!(attr.id(), Token::from("buffer"));
        assert_eq!(attr.typ(), Token::from("@buffer"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get().unwrap() {
            Variant::Buffer(val) => assert_eq!(&*val, b"123456789"),
            _ => panic!(),
        };
    }

    #[test]
    fn string() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                Ok(Variant::String(
                    from_utf8(&data).unwrap().to_string().into_boxed_str(),
                ))
            }
        }
        let class = AttrClass::builder("string")
            .typ("@string")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"123456789"[..])));
        assert_eq!(attr.id(), Token::from("string"));
        assert_eq!(attr.typ(), Token::from("@string"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get().unwrap() {
            Variant::String(val) => assert_eq!(&*val, "123456789"),
            _ => panic!(),
        };
    }

    #[test]
    fn slice() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, data: &ByteSlice) -> Result<Variant> {
                data.try_get(0..3).map(Variant::Slice)
            }
        }
        let class = AttrClass::builder("slice")
            .typ("@slice")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"123456789"[..])));
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), Token::from("@slice"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get().unwrap() {
            Variant::Slice(val) => assert_eq!(val, ByteSlice::from(&b"123"[..])),
            _ => panic!(),
        };
    }

    #[test]
    fn error() {
        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _attr: &Attr, _: &ByteSlice) -> Result<Variant> {
                Err(Error::new(ErrorKind::Other, "oh no!"))
            }
        }
        let class = AttrClass::builder("slice")
            .typ("@slice")
            .range(0..6)
            .cast(&TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, Some(ByteSlice::from(&b"123456789"[..])));
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), Token::from("@slice"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get() {
            Err(err) => assert_eq!(err.description(), "oh no!"),
            _ => panic!(),
        };
    }
}
