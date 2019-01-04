use crate::{
    cast::{Cast, Nil, Typed},
    env,
    fixed::Fixed,
    layer::Layer,
    metadata::Metadata,
    result::Result,
    slice::{ByteSlice, TryGet},
    string::SafeString,
    token::Token,
    variant::Variant,
    vec::SafeVec,
};
use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use failure::format_err;
use std::{
    cell::Cell,
    fmt,
    io::{self, Cursor},
    marker::PhantomData,
    mem,
    ops::{Deref, Range},
    slice,
};

#[derive(Debug, Clone)]
pub struct Attr2Context<T> {
    pub id: String,
    pub path: String,
    pub typ: String,
    pub name: &'static str,
    pub description: &'static str,
    pub little_endian: bool,
    pub bit_size: usize,
    pub bit_offset: usize,
    pub aliases: Vec<String>,
    pub func_map: fn(T) -> T,
    pub func_cond: fn(T) -> bool,
}

pub struct Attr2Functor<T> {
    ctx: Attr2Context<T>,
    func_map: Box<Fn(&Attr, &ByteSlice) -> io::Result<T> + Send + Sync>,
    func_var: Box<Fn(&Attr, &ByteSlice) -> io::Result<Variant> + Send + Sync>,
    func_cond: Box<Fn(&Attr, &ByteSlice) -> bool + Send + Sync>,
}

impl<T> Default for Attr2Context<T> {
    fn default() -> Self {
        Self {
            id: Default::default(),
            path: Default::default(),
            typ: Default::default(),
            name: Default::default(),
            description: Default::default(),
            little_endian: Default::default(),
            bit_size: Default::default(),
            bit_offset: Default::default(),
            aliases: Default::default(),
            func_map: |x| x,
            func_cond: |_| true,
        }
    }
}

pub struct Attr2Cast {
    func: Box<Fn(&Attr, &ByteSlice) -> io::Result<Variant> + Send + Sync>,
}

impl Cast for Attr2Cast {
    fn cast(&self, attr: &Attr, data: &ByteSlice) -> io::Result<Variant> {
        (self.func)(attr, data)
    }
}

pub trait Attr2Field {
    type Output: Into<Variant>;
    fn context() -> Attr2Context<Self::Output>;
    fn new(ctx: Attr2Functor<Self::Output>) -> Self;
    fn build(ctx: &Attr2Context<Self::Output>) -> Attr2Functor<Self::Output>;
}

pub struct Enum2Field<F, E> {
    phantom: PhantomData<F>,
    class: Fixed<AttrClass>,
    func: Box<Fn(&Attr, &ByteSlice) -> io::Result<E> + Send + Sync>,
}

impl<F: Attr2Field, E> Attr2Field for Enum2Field<F, E>
where
    F::Output: 'static + Into<E>,
{
    type Output = F::Output;

    fn context() -> Attr2Context<Self::Output> {
        F::context()
    }

    fn new(ctx: Attr2Functor<Self::Output>) -> Self {
        Self {
            phantom: PhantomData,
            class: Fixed::new(AttrClass::builder("").build()),
            func: Box::new(move |attr, data| (ctx.func_map)(attr, data).map(|x| x.into())),
        }
    }

    fn build(ctx: &Attr2Context<Self::Output>) -> Attr2Functor<Self::Output> {
        F::build(ctx)
    }
}

impl<F: Attr2Field, E> Enum2Field<F, E>
where
    F::Output: Into<E>,
{
    pub fn try_get_range(&self, layer: &Layer, range: Range<usize>) -> io::Result<E> {
        let attr = Attr::new(&self.class, range, layer.data());
        (self.func)(&attr, &layer.data())
    }

    pub fn try_get(&self, layer: &Layer) -> io::Result<E> {
        self.try_get_range(layer, self.class.range())
    }
}

impl Attr2Field for u8 {
    type Output = Self;

    fn context() -> Attr2Context<Self::Output> {
        Attr2Context {
            bit_size: mem::size_of::<Self>(),
            ..Default::default()
        }
    }

    fn new(_ctx: Attr2Functor<Self::Output>) -> Self {
        0
    }

    fn build(ctx: &Attr2Context<Self::Output>) -> Attr2Functor<Self::Output> {
        let mctx = ctx.clone();
        let func_map: Box<Fn(&Attr, &ByteSlice) -> io::Result<Self::Output> + Send + Sync> =
            Box::new(move |attr, data| {
                Cursor::new(data.try_get(attr.range())?)
                    .read_u8()
                    .map(mctx.func_map)
            });

        let mctx = ctx.clone();
        let func_var: Box<Fn(&Attr, &ByteSlice) -> io::Result<Variant> + Send + Sync> =
            Box::new(move |attr, data| {
                Cursor::new(data.try_get(attr.range())?)
                    .read_u8()
                    .map(mctx.func_map)
                    .map(|x| x.into())
            });

        let mctx = ctx.clone();
        let func_cond: Box<Fn(&Attr, &ByteSlice) -> bool + Send + Sync> =
            Box::new(move |attr, data| {
                data.try_get(attr.range())
                    .ok()
                    .and_then(|data| Cursor::new(data).read_u8().ok())
                    .map(mctx.func_map)
                    .map(mctx.func_cond)
                    .is_some()
            });

        Attr2Functor {
            ctx: ctx.clone(),
            func_map,
            func_var,
            func_cond,
        }

        /*
        let bit_size = ctx.bit_size;
        AttrClass::builder(&format!("{}.{}", ctx.path, ctx.id))
            .cast(Attr2Cast{ func: map_var })
            .aliases(ctx.aliases.clone())
            .name(ctx.name)
            .description(ctx.description)
            .typ(&ctx.typ)
            .bit_range(0, ctx.bit_offset..(ctx.bit_offset + bit_size))
        */
    }
}

#[derive(Debug, Clone, Default)]
pub struct AttrContext {
    pub path: String,
    pub typ: String,
    pub name: &'static str,
    pub description: &'static str,
    pub bit_offset: usize,
    pub detached: bool,
    pub aliases: Vec<String>,
}

pub trait AttrField {
    type I;

    fn class(
        &self,
        ctx: &AttrContext,
        bit_size: usize,
        filter: Option<fn(Self::I) -> Variant>,
    ) -> AttrClassBuilder;
}

pub trait SizedField {
    fn bit_size(&self) -> usize;
    fn byte_size(&self) -> usize {
        (self.bit_size() + 7) / 8
    }
}

/// An attribute object.
#[repr(C)]
pub struct Attr<'a> {
    class: &'a AttrClass,
    range: Range<usize>,
    data: ByteSlice,
}

impl<'a> fmt::Debug for Attr<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Attr {:?}", self.id())
    }
}

impl<'a> Attr<'a> {
    pub fn new(class: &'a AttrClass, range: Range<usize>, data: ByteSlice) -> Attr<'a> {
        Attr { class, range, data }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.class.id()
    }

    pub fn is_match(&self, id: Token) -> bool {
        self.class.is_match(id)
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

impl<T: 'static + Into<Variant> + Send + Sync + Clone> Typed for Const<T> {
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
    aliases: Vec<Token>,
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
    pub fn cast<T: Cast>(mut self, cast: T) -> AttrClassBuilder {
        self.cast = Box::new(cast);
        self
    }

    pub fn alias<T: Into<Token>>(mut self, id: T) -> AttrClassBuilder {
        self.aliases.push(id.into());
        self
    }

    pub fn aliases<T: Into<Token>>(mut self, aliases: Vec<T>) -> AttrClassBuilder {
        let mut aliases = aliases.into_iter().map(|s| s.into()).collect();
        self.aliases.append(&mut aliases);
        self
    }

    pub fn child<C: Into<Fixed<AttrClass>>>(mut self, class: C) -> AttrClassBuilder {
        self.children.push(class.into());
        self
    }

    pub fn add_children<C: Into<Fixed<AttrClass>>>(mut self, children: Vec<C>) -> AttrClassBuilder {
        let mut children = children.into_iter().map(|s| s.into()).collect();
        self.children.append(&mut children);
        self
    }

    pub fn children(&self) -> &[Fixed<AttrClass>] {
        &self.children
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
            is_match: abi_is_match,
            get_typ: abi_typ,
            get: abi_get,
            id: self.id,
            typ: self.typ,
            meta: self.meta,
            range: self.range,
            children: self.children,
            aliases: self.aliases,
            cast: self.cast,
        }
    }
}

/// An attribute class.
#[repr(C)]
pub struct AttrClass {
    get_id: extern "C" fn(class: *const AttrClass) -> Token,
    is_match: extern "C" fn(class: *const AttrClass, id: Token) -> u8,
    get_typ: extern "C" fn(class: *const AttrClass) -> Token,
    get: extern "C" fn(*const Attr, *mut *const u8, u64, *mut i64, *mut SafeString) -> ValueType,
    id: Token,
    typ: Token,
    meta: Metadata,
    range: Range<usize>,
    children: Vec<Fixed<AttrClass>>,
    aliases: Vec<Token>,
    cast: Box<Cast>,
}

impl fmt::Debug for AttrClass {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("AttrClass")
            .field("id", &self.id)
            .field("name", &self.meta.name())
            .field("description", &self.meta.description())
            .field("typ", &self.typ)
            .field("range", &self.range)
            .field("aliases", &self.aliases)
            .field("children", &self.children)
            .finish()
    }
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
            aliases: Vec::new(),
            cast: Box::new(Const(true)),
        }
    }

    pub fn id(&self) -> Token {
        (self.get_id)(self)
    }

    pub fn is_match(&self, id: Token) -> bool {
        (self.is_match)(self, id) != 0
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
        let root = Attr::new(attr, range.clone(), *data);
        let mut attrs = vec![root];
        for child in &attr.children {
            let offset = range.start as isize - attr.bit_range().start as isize;
            let range = (child.bit_range().start as isize + offset) as usize
                ..(child.bit_range().end as isize + offset) as usize;
            attrs.append(&mut AttrClass::expand(&child, &data, Some(range)));
        }
        attrs
    }

    pub fn try_get(&self, layer: &Layer) -> Result<Variant> {
        self.try_get_range(layer, self.range())
    }

    pub fn try_get_range(&self, layer: &Layer, range: Range<usize>) -> Result<Variant> {
        let bit_offset = self.bit_range().start;
        let range = (self.bit_range().start - bit_offset + range.start * 8)
            ..(self.bit_range().end - bit_offset + range.end * 8);
        let attr = Attr::new(self, range, layer.data());
        self.try_get_attr(&attr)
    }

    fn try_get_attr(&self, attr: &Attr) -> Result<Variant> {
        let data = attr.data;
        let mut buf: *const u8 = data.as_ptr();
        let mut num = 0;
        let mut err = SafeString::new();
        let typ = (self.get)(attr, &mut buf, data.len() as u64, &mut num, &mut err);
        match typ {
            ValueType::Error => Err(format_err!("{}", err)),
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

extern "C" fn abi_is_match(class: *const AttrClass, id: Token) -> u8 {
    let class = unsafe { &(*class) };
    if class.id == id || class.aliases.iter().any(|&x| x == id) {
        1
    } else {
        0
    }
}

extern "C" fn abi_typ(class: *const AttrClass) -> Token {
    unsafe { (*class).typ }
}

extern "C" fn abi_get(
    attr: *const Attr,
    data: *mut *const u8,
    len: u64,
    num: *mut i64,
    err: *mut SafeString,
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
            unsafe { *err = SafeString::from(&format!("{}", e)) }
            ValueType::Error
        }
    }
}

pub struct Node<T, U = Nil> {
    node: T,
    fields: U,
    class: Cell<Option<Fixed<AttrClass>>>,
}

impl<T, U> Node<T, U> {
    pub fn class(&self) -> Fixed<AttrClass> {
        self.class.get().unwrap()
    }

    pub fn try_get_range(&self, layer: &Layer, range: Range<usize>) -> Result<Variant> {
        self.class().try_get_range(layer, range)
    }

    pub fn try_get(&self, layer: &Layer) -> Result<Variant> {
        self.class().try_get(layer)
    }
}

impl<T, U> Deref for Node<T, U> {
    type Target = U;

    fn deref(&self) -> &U {
        &self.fields
    }
}

impl<I: Into<Variant>, J: Into<Variant>, T: AttrField<I = I>, U: AttrField<I = J>> AttrField
    for Node<T, U>
{
    type I = I;

    fn class(
        &self,
        ctx: &AttrContext,
        bit_size: usize,
        filter: Option<fn(Self::I) -> Variant>,
    ) -> AttrClassBuilder {
        let node = self.node.class(ctx, bit_size, filter);
        let fields = self.fields.class(ctx, bit_size, None);

        if self.class.get().is_none() {
            self.class.set(Some(Fixed::new(
                self.node
                    .class(ctx, bit_size, filter)
                    .add_children(fields.children().to_vec())
                    .build(),
            )))
        }

        node.add_children(fields.children().to_vec())
    }
}

impl<T: SizedField, U> SizedField for Node<T, U> {
    fn bit_size(&self) -> usize {
        self.node.bit_size()
    }
}

impl<T: Default, U: Default> Default for Node<T, U> {
    fn default() -> Self {
        Self {
            node: T::default(),
            fields: U::default(),
            class: Cell::new(None),
        }
    }
}

pub trait EnumField<T: Into<Variant>> {
    fn class_enum<C: Typed<Output = T> + Clone>(
        &self,
        ctx: &AttrContext,
        bit_size: usize,
        cast: C,
    ) -> AttrClassBuilder;
}

pub struct EnumNode<T, U> {
    node: T,
    fields: U,
    class: Cell<Option<Fixed<AttrClass>>>,
}

impl<I: Into<Variant>, T: AttrField<I = I> + Typed<Output = I> + Clone, U: EnumField<I>> AttrField
    for EnumNode<T, U>
{
    type I = I;

    fn class(
        &self,
        ctx: &AttrContext,
        bit_size: usize,
        filter: Option<fn(Self::I) -> Variant>,
    ) -> AttrClassBuilder {
        let fields = self.fields.class_enum(ctx, bit_size, self.node.clone());

        if self.class.get().is_none() {
            self.class.set(Some(Fixed::new(
                self.node
                    .class(ctx, bit_size, filter)
                    .add_children(fields.children().to_vec())
                    .build(),
            )))
        }

        fields
    }
}

impl<I: Into<Variant> + Into<U>, T: Typed<Output = I>, U: EnumField<I>> EnumNode<T, U> {
    pub fn try_get_range(&self, layer: &Layer, range: Range<usize>) -> Result<U> {
        let class = self.class.get().unwrap();

        let bit_offset = class.bit_range().start;
        let range = (class.bit_range().start - bit_offset + range.start * 8)
            ..(class.bit_range().end - bit_offset + range.end * 8);

        let root = Attr::new(&class, range, layer.data());
        match Typed::cast(&self.node, &root, &layer.data()) {
            Ok(data) => Ok(data.into()),
            Err(err) => Err(format_err!("{}", err)),
        }
    }

    pub fn try_get(&self, layer: &Layer) -> Result<U> {
        let class = self.class.get().unwrap();
        self.try_get_range(layer, class.range())
    }
}

impl<T: SizedField, U> SizedField for EnumNode<T, U> {
    fn bit_size(&self) -> usize {
        self.node.bit_size()
    }
}

impl<T, U> EnumNode<T, U> {
    pub fn class(&self) -> Fixed<AttrClass> {
        self.class.get().unwrap()
    }
}

impl<T: Default, U: Default> Default for EnumNode<T, U> {
    fn default() -> Self {
        Self {
            node: T::default(),
            fields: U::default(),
            class: Cell::new(None),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        attr::{Attr, AttrClass},
        cast::Cast,
        slice::{ByteSlice, TryGet},
        token::Token,
        variant::Variant,
    };
    use std::{
        io::{Error, ErrorKind, Result},
        str::from_utf8,
    };

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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..8, ByteSlice::from(&[1][..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"123456789"[..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"-123456789"[..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"123456789"[..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"123456789"[..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"123456789"[..]));
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
            .cast(TestCast {})
            .build();
        let attr = Attr::new(&class, 0..48, ByteSlice::from(&b"123456789"[..]));
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), Token::from("@slice"));
        assert_eq!(attr.range(), 0..6);

        match attr.try_get() {
            Err(err) => assert_eq!(format!("{}", err), "oh no!"),
            _ => panic!(),
        };
    }
}
