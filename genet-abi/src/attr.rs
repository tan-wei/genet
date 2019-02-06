use crate::{
    bytes::{Bytes, TryGet},
    fixed::Fixed,
    layer::Layer,
    metadata::Metadata,
    result::Result,
    string::SafeString,
    token::Token,
    variant::{TryInto, Variant},
};
use byteorder::{BigEndian, LittleEndian, ReadBytesExt, WriteBytesExt};
use failure::format_err;
use num_bigint::BigInt;
use std::{
    any::TypeId,
    fmt,
    io::{Cursor, Write},
    marker::PhantomData,
    mem,
    ops::{Deref, DerefMut, Range},
    slice, str,
};

#[derive(Clone)]
pub struct AttrContext<I, O> {
    pub id: String,
    pub path: String,
    pub typ: String,
    pub name: &'static str,
    pub description: &'static str,
    pub little_endian: bool,
    pub bit_size: usize,
    pub bit_offset: usize,
    pub aliases: Vec<String>,
    pub func_map: fn(I) -> Result<O>,
    pub func_cond: fn(&O) -> bool,
}

#[derive(Default, Clone)]
pub struct WriterContext {
    pub little_endian: bool,
    pub bit_size: usize,
    pub bit_offset: usize,
}

pub trait Mapper<O>: Send + Sync {
    fn invoke(&self, attr: &Attr, data: &Bytes) -> Result<O>;
    fn box_clone(&self) -> Box<Mapper<O>>;
}

impl<O> Clone for Box<Mapper<O>> {
    fn clone(&self) -> Box<Mapper<O>> {
        self.box_clone()
    }
}

pub struct MapFunc<O: 'static, F: 'static + Fn(&Attr, &Bytes) -> Result<O> + Send + Sync + Clone>(
    F,
);

impl<O: 'static, F: 'static + Fn(&Attr, &Bytes) -> Result<O> + Send + Sync + Clone> MapFunc<O, F> {
    pub fn wrap(func: F) -> Box<Mapper<O>> {
        Box::new(MapFunc(func))
    }
}

impl<O: 'static, F: 'static + Fn(&Attr, &Bytes) -> Result<O> + Send + Sync + Clone> Mapper<O>
    for MapFunc<O, F>
{
    fn invoke(&self, attr: &Attr, data: &Bytes) -> Result<O> {
        (self.0)(attr, data)
    }

    fn box_clone(&self) -> Box<Mapper<O>> {
        Box::new(MapFunc(self.0.clone()))
    }
}

pub struct AttrFunctor<I: 'static, O: 'static + Into<Variant>> {
    pub ctx: AttrContext<I, O>,
    pub func_map: Box<Mapper<O>>,
}

impl<I: 'static, O: 'static + Into<Variant>> Into<AttrClassBuilder> for AttrFunctor<I, O> {
    fn into(self) -> AttrClassBuilder {
        let func_map = self.func_map;
        let func_cond = self.ctx.func_cond;
        AttrClass::builder(format!("{}.{}", self.ctx.path, self.ctx.id).trim_matches('.'))
            .cast(move |attr, data| {
                func_map.invoke(attr, data).map(|x| {
                    if (func_cond)(&x) {
                        x.into()
                    } else {
                        Variant::Nil
                    }
                })
            })
            .aliases(self.ctx.aliases.clone())
            .name(self.ctx.name)
            .description(self.ctx.description)
            .typ(&self.ctx.typ)
            .bit_range(self.ctx.bit_offset..(self.ctx.bit_offset + self.ctx.bit_size))
    }
}

impl<I, O> Default for AttrContext<I, O>
where
    I: Into<Variant>,
    Variant: TryInto<O>,
{
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
            func_map: |x| x.into().try_into(),
            func_cond: |_| true,
        }
    }
}

pub struct Cast<I, O> {
    phantom: PhantomData<(I, O)>,
}

impl<I: AttrField, O: AttrField> AttrField for Cast<I, O>
where
    I::Input: 'static + Into<Variant>,
    I::Output: 'static + Into<I::Input>,
    O::Input: 'static,
    Variant: TryInto<O::Input>,
{
    type Input = I::Input;
    type Output = O::Input;

    fn context() -> AttrContext<Self::Input, Self::Output> {
        AttrContext {
            bit_size: I::context().bit_size,
            ..AttrContext::default()
        }
    }

    fn new(_ctx: &AttrContext<Self::Input, Self::Output>) -> Self {
        Self {
            phantom: PhantomData,
        }
    }

    fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder> {
        vec![Self::build(ctx).into()]
    }

    fn build(
        ctx: &AttrContext<Self::Input, Self::Output>,
    ) -> AttrFunctor<Self::Input, Self::Output> {
        let mctx_fm = ctx.func_map;

        let mut subctx = I::context();
        subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
        subctx.bit_offset = ctx.bit_offset;
        subctx.bit_size = ctx.bit_size;
        subctx.little_endian = ctx.little_endian;
        let func = I::build(&subctx);

        let func_map = func.func_map;
        let func_map = MapFunc::wrap(move |attr, data| {
            func_map
                .invoke(attr, data)
                .map(|x| x.into())
                .and_then(mctx_fm)
        });

        let mut subctx = Self::context();
        subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
        subctx.typ = ctx.typ.clone();
        subctx.aliases = ctx.aliases.clone();
        subctx.name = ctx.name;
        subctx.description = ctx.description;
        subctx.bit_offset = ctx.bit_offset;
        subctx.bit_size = ctx.bit_size;
        subctx.little_endian = ctx.little_endian;

        AttrFunctor {
            ctx: subctx,
            func_map,
        }
    }
}

pub trait AttrField {
    type Input: Into<Variant>;
    type Output: Into<Variant>;
    fn context() -> AttrContext<Self::Input, Self::Output>;
    fn new(ctx: &AttrContext<Self::Input, Self::Output>) -> Self;
    fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder>;
    fn build(
        ctx: &AttrContext<Self::Input, Self::Output>,
    ) -> AttrFunctor<Self::Input, Self::Output>;
    fn write(&self, _ctx: &WriterContext, _data: &mut [u8]) -> Result<()> {
        Ok(())
    }
}

pub struct Node<F: AttrField, C: AttrField = F> {
    data: C,
    class: Vec<Fixed<AttrClass>>,
    func: Box<Fn(&Attr, &Bytes) -> Result<F::Output> + Send + Sync>,
}

impl<F: AttrField, C: AttrField> Node<F, C> {
    pub fn get_range(&self, layer: &Layer, byte_range: Range<usize>) -> Result<F::Output> {
        let class = self.class[0];
        let bit_offset = class.bit_range().start;
        let range = (class.bit_range().start - bit_offset + byte_range.start * 8)
            ..(class.bit_range().end - bit_offset + byte_range.end * 8);
        let attr = Attr::new(&class, range, layer.data());
        (self.func)(&attr, &layer.data())
    }

    pub fn get(&self, layer: &Layer) -> Result<F::Output> {
        self.get_range(layer, self.byte_range())
    }

    pub fn byte_range(&self) -> Range<usize> {
        self.class[0].byte_range()
    }

    pub fn bit_range(&self) -> Range<usize> {
        self.class[0].bit_range()
    }
}

impl<F: AttrField, C: AttrField> AsRef<[Fixed<AttrClass>]> for Node<F, C> {
    fn as_ref(&self) -> &[Fixed<AttrClass>] {
        &self.class
    }
}

impl<F: AttrField, C: AttrField> Deref for Node<F, C> {
    type Target = C;

    fn deref(&self) -> &C {
        &self.data
    }
}

impl<F: AttrField, C: AttrField> DerefMut for Node<F, C> {
    fn deref_mut(&mut self) -> &mut C {
        &mut self.data
    }
}

impl<F: AttrField, C: AttrField> AttrField for Node<F, C>
where
    F: 'static,
    C: 'static,
    F::Input: 'static,
    F::Output: 'static,
{
    type Input = F::Input;
    type Output = F::Output;

    fn context() -> AttrContext<Self::Input, Self::Output> {
        F::context()
    }

    fn new(ctx: &AttrContext<Self::Input, Self::Output>) -> Self {
        let func = Self::build(ctx);
        let mut subctx = C::context();
        subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
        subctx.bit_offset = ctx.bit_offset;
        subctx.little_endian = ctx.little_endian;

        let class = Self::class(ctx)
            .into_iter()
            .map(|attr| Fixed::new(attr.build()))
            .collect();

        Self {
            data: C::new(&subctx),
            class,
            func: Box::new(move |attr, data| func.func_map.invoke(attr, data)),
        }
    }

    fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder> {
        let mut subctx = C::context();
        subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
        subctx.bit_offset = ctx.bit_offset;
        subctx.little_endian = ctx.little_endian;
        let mut v = F::class(ctx);
        if TypeId::of::<F>() != TypeId::of::<C>() {
            let mut children = C::class(&subctx).into_iter().skip(1).collect();
            v.append(&mut children);
        }
        v
    }

    fn build(
        ctx: &AttrContext<Self::Input, Self::Output>,
    ) -> AttrFunctor<Self::Input, Self::Output> {
        F::build(ctx)
    }

    fn write(&self, ctx: &WriterContext, data: &mut [u8]) -> Result<()> {
        self.data.write(ctx, data)
    }
}

pub trait EnumType {
    type Output;
    fn class<T: 'static + AttrField<Output = E>, E: 'static + Into<Variant> + Into<Self::Output>>(
        ctx: &AttrContext<T::Input, E>,
    ) -> Vec<AttrClassBuilder>;
}

pub struct Enum<F: AttrField, E> {
    data: F,
    class: Vec<Fixed<AttrClass>>,
    func: Box<Fn(&Attr, &Bytes) -> Result<E>>,
}

impl<F: AttrField, E: EnumType> AsRef<[Fixed<AttrClass>]> for Enum<F, E> {
    fn as_ref(&self) -> &[Fixed<AttrClass>] {
        &self.class
    }
}

impl<F: 'static + AttrField, E: 'static + EnumType<Output = E>> AttrField for Enum<F, E>
where
    F::Input: 'static,
    F::Output: 'static + Into<E>,
{
    type Input = F::Input;
    type Output = F::Output;

    fn context() -> AttrContext<Self::Input, Self::Output> {
        AttrContext {
            typ: "@enum".into(),
            ..F::context()
        }
    }

    fn new(ctx: &AttrContext<Self::Input, Self::Output>) -> Self {
        let func = Self::build(ctx);
        let class = Self::class(ctx)
            .into_iter()
            .map(|attr| Fixed::new(attr.build()))
            .collect();
        Self {
            data: F::new(ctx),
            class,
            func: Box::new(move |attr, data| func.func_map.invoke(attr, data).map(|x| x.into())),
        }
    }

    fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder> {
        let mut subctx = F::context();
        subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
        subctx.bit_offset = ctx.bit_offset;
        subctx.bit_size = ctx.bit_size;
        subctx.little_endian = ctx.little_endian;
        subctx.func_map = ctx.func_map;
        subctx.func_cond = ctx.func_cond;

        let mut v = F::class(ctx);
        v.append(&mut E::class::<Self, Self::Output>(&subctx));
        v
    }

    fn build(
        ctx: &AttrContext<Self::Input, Self::Output>,
    ) -> AttrFunctor<Self::Input, Self::Output> {
        F::build(ctx)
    }

    fn write(&self, ctx: &WriterContext, data: &mut [u8]) -> Result<()> {
        self.data.write(ctx, data)
    }
}

impl<F: AttrField, E> Deref for Enum<F, E> {
    type Target = F;

    fn deref(&self) -> &Self::Target {
        &self.data
    }
}

impl<F: AttrField, E> DerefMut for Enum<F, E> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.data
    }
}

impl<F: AttrField, E> Enum<F, E>
where
    F::Output: Into<E>,
{
    pub fn get_range(&self, layer: &Layer, byte_range: Range<usize>) -> Result<E> {
        let class = self.class[0];
        let bit_offset = class.bit_range().start;
        let range = (class.bit_range().start - bit_offset + byte_range.start * 8)
            ..(class.bit_range().end - bit_offset + byte_range.end * 8);
        let attr = Attr::new(&class, range, layer.data());
        (self.func)(&attr, &layer.data())
    }

    pub fn get(&self, layer: &Layer) -> Result<E> {
        self.get_range(layer, self.byte_range())
    }

    pub fn byte_range(&self) -> Range<usize> {
        self.class[0].byte_range()
    }

    pub fn bit_range(&self) -> Range<usize> {
        self.class[0].bit_range()
    }
}

macro_rules! define_field {
    ($t:ty, $size:expr, $little:block, $big:block, $little_write:block, $big_write:block) => {
        impl AttrField for $t {
            type Input = Self;
            type Output = Self;

            fn context() -> AttrContext<Self::Input, Self::Output> {
                AttrContext {
                    bit_size: $size,
                    ..Default::default()
                }
            }

            fn new(_ctx: &AttrContext<Self::Input, Self::Output>) -> Self {
                Default::default()
            }

            fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder> {
                vec![Self::build(ctx).into()]
            }

            fn build(
                ctx: &AttrContext<Self::Input, Self::Output>,
            ) -> AttrFunctor<Self::Input, Self::Output> {
                let parse: fn(&Bytes, Range<usize>) -> Result<Self::Output> =
                    if ctx.little_endian { $little } else { $big };

                let func_map = ctx.func_map;
                let func_map = MapFunc::wrap(move |attr, data| {
                    parse(data, attr.byte_range()).and_then(func_map)
                });

                AttrFunctor {
                    ctx: ctx.clone(),
                    func_map,
                }
            }

            fn write(&self, ctx: &WriterContext, data: &mut [u8]) -> Result<()> {
                let func = $little_write;
                func(self, Cursor::new(data)).map_err(|e| e.into())
            }
        }
    };
}

define_field!(
    u8,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u8()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u8()
                .map_err(|e| e.into())
        }
    },
    { |value: &u8, mut data: Cursor<&mut [u8]>| data.write_u8(*value) },
    { |value: &u8, mut data: Cursor<&mut [u8]>| data.write_u8(*value) }
);

define_field!(
    i8,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i8()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i8()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    u16,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u16::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u16::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    i16,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i16::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i16::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    u32,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u32::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u32::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    i32,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i32::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i32::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    u64,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u64::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u64::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    i64,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i64::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_i64::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    f32,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_f32::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_f32::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    f64,
    (mem::size_of::<Self>() * 8),
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_f64::<LittleEndian>()
                .map_err(|e| e.into())
        }
    },
    {
        |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_f64::<BigEndian>()
                .map_err(|e| e.into())
        }
    },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) },
    { |value, mut data: Cursor<&mut [u8]>| data.write_u8(1) }
);

define_field!(
    Bytes,
    8,
    { |data: &Bytes, range: Range<usize>| data.get(range) },
    { |data: &Bytes, range: Range<usize>| data.get(range) },
    { |value, mut data: Cursor<&mut [u8]>| data.write(value).map(|_| ()) },
    { |value, mut data: Cursor<&mut [u8]>| data.write(value).map(|_| ()) }
);

impl AttrField for bool {
    type Input = bool;
    type Output = bool;

    fn context() -> AttrContext<Self::Input, Self::Output> {
        AttrContext {
            bit_size: 1,
            ..Default::default()
        }
    }

    fn new(_ctx: &AttrContext<Self::Input, Self::Output>) -> Self {
        false
    }

    fn class(ctx: &AttrContext<Self::Input, Self::Output>) -> Vec<AttrClassBuilder> {
        vec![Self::build(ctx).into()]
    }

    fn build(
        ctx: &AttrContext<Self::Input, Self::Output>,
    ) -> AttrFunctor<Self::Input, Self::Output> {
        let parse = |data: &Bytes, range: Range<usize>| {
            Cursor::new(data.get(range)?)
                .read_u8()
                .map_err(|e| e.into())
        };

        let func_map = ctx.func_map;
        let func_map = MapFunc::wrap(move |attr, data| {
            parse(data, attr.byte_range())
                .map(|byte| (byte & (0b1000_0000 >> (attr.bit_range().start % 8))) != 0)
                .and_then(func_map)
        });

        AttrFunctor {
            ctx: ctx.clone(),
            func_map,
        }
    }
}

/// An attribute object.
#[repr(C)]
pub struct Attr<'a> {
    class: &'a AttrClass,
    bit_range: Range<usize>,
    data: Bytes,
}

impl<'a> fmt::Debug for Attr<'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Attr {:?}", self.id())
    }
}

impl<'a> Attr<'a> {
    pub fn new(class: &'a AttrClass, bit_range: Range<usize>, data: Bytes) -> Attr<'a> {
        Attr {
            class,
            bit_range,
            data,
        }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.class.id()
    }

    pub fn path(&self) -> &str {
        self.class.path()
    }

    pub fn is_match(&self, id: Token) -> bool {
        self.class.is_match(id)
    }

    /// Returns the type of self.
    pub fn typ(&self) -> &str {
        self.class.typ()
    }

    /// Returns the byte range of self.
    pub fn byte_range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    /// Returns the bit range of self.
    pub fn bit_range(&self) -> Range<usize> {
        self.bit_range.clone()
    }

    /// Returns the attribute value.
    pub fn get(&self) -> Result<Variant> {
        self.class.get_attr(self)
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
    Buffer = 5,
    BigInt = 6,
    Bytes = 7,
}

/// A builder object for AttrClass.
pub struct AttrClassBuilder {
    id: Token,
    path: String,
    typ: String,
    meta: Metadata,
    bit_range: Range<usize>,
    aliases: Vec<Token>,
    cast: Box<Fn(&Attr, &Bytes) -> Result<Variant> + Send + Sync>,
}

impl AttrClassBuilder {
    /// Sets a type of AttrClass.
    pub fn typ(mut self, typ: &str) -> AttrClassBuilder {
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
    pub fn byte_range(mut self, range: Range<usize>) -> AttrClassBuilder {
        self.bit_range = (range.start * 8)..(range.end * 8);
        self
    }

    /// Sets a bit range of Attr.
    pub fn bit_range(mut self, range: Range<usize>) -> AttrClassBuilder {
        self.bit_range = range;
        self
    }

    /// Sets a cast of AttrClass.
    pub fn cast<F: 'static + Fn(&Attr, &Bytes) -> Result<Variant> + Send + Sync>(
        mut self,
        cast: F,
    ) -> AttrClassBuilder {
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

    /// Sets a constant value of AttrClass.
    pub fn value<T: 'static + Into<Variant> + Send + Sync + Clone>(
        mut self,
        value: T,
    ) -> AttrClassBuilder {
        let value: Variant = value.into();
        self.cast = Box::new(move |_, _| Ok(value.clone()));
        self
    }

    /// Builds a new AttrClass.
    pub fn build(self) -> AttrClass {
        AttrClass {
            get_id: abi_id,
            get_path: abi_path,
            is_match: abi_is_match,
            get_typ: abi_typ,
            get: abi_get,
            id: self.id,
            path: self.path,
            typ: self.typ,
            meta: self.meta,
            bit_range: self.bit_range,
            aliases: self.aliases,
            cast: self.cast,
        }
    }
}

/// An attribute class.
#[repr(C)]
pub struct AttrClass {
    get_id: extern "C" fn(class: *const AttrClass) -> Token,
    get_path: extern "C" fn(class: *const AttrClass, len: *mut u64) -> *const u8,
    is_match: extern "C" fn(class: *const AttrClass, id: Token) -> u8,
    get_typ: extern "C" fn(class: *const AttrClass, len: *mut u64) -> *const u8,
    get: extern "C" fn(*const Attr, *mut *const u8, u64, *mut i64, *mut SafeString) -> ValueType,
    id: Token,
    path: String,
    typ: String,
    meta: Metadata,
    bit_range: Range<usize>,
    aliases: Vec<Token>,
    cast: Box<Fn(&Attr, &Bytes) -> Result<Variant> + Send + Sync>,
}

impl fmt::Debug for AttrClass {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("AttrClass")
            .field("id", &self.id)
            .field("name", &self.meta.name())
            .field("description", &self.meta.description())
            .field("typ", &self.typ)
            .field("bit_range", &self.bit_range)
            .field("aliases", &self.aliases)
            .finish()
    }
}

impl AttrClass {
    /// Creates a new builder object for AttrClass.
    pub fn builder(id: &str) -> AttrClassBuilder {
        AttrClassBuilder {
            id: Token::from(id),
            path: id.to_string(),
            typ: String::new(),
            meta: Metadata::new(),
            bit_range: 0..0,
            aliases: Vec::new(),
            cast: Box::new(|_, _| Ok(Variant::Nil)),
        }
    }

    pub fn id(&self) -> Token {
        (self.get_id)(self)
    }

    pub fn path(&self) -> &str {
        let mut len = 0;
        let data = (self.get_path)(self, &mut len);
        unsafe { str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize)) }
    }

    pub fn is_match(&self, id: Token) -> bool {
        (self.is_match)(self, id) != 0
    }

    pub fn typ(&self) -> &str {
        let mut len = 0;
        let data = (self.get_typ)(self, &mut len);
        unsafe { str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize)) }
    }

    pub fn byte_range(&self) -> Range<usize> {
        let range = self.bit_range();
        (range.start / 8)..((range.end + 7) / 8)
    }

    pub fn bit_range(&self) -> Range<usize> {
        self.bit_range.clone()
    }

    pub fn get(&self, layer: &Layer) -> Result<Variant> {
        self.get_range(layer, self.byte_range())
    }

    pub fn get_range(&self, layer: &Layer, byte_range: Range<usize>) -> Result<Variant> {
        let bit_offset = self.bit_range().start;
        let range = (self.bit_range().start - bit_offset + byte_range.start * 8)
            ..(self.bit_range().end - bit_offset + byte_range.end * 8);
        let attr = Attr::new(self, range, layer.data());
        self.get_attr(&attr)
    }

    fn get_attr(&self, attr: &Attr) -> Result<Variant> {
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
                Box::from_raw(buf as *mut u8);
                Ok(Variant::Buffer(b))
            },
            ValueType::BigInt => unsafe {
                let len: u64 = mem::transmute_copy(&num);
                let b = slice::from_raw_parts(buf, len as usize);
                Box::from_raw(buf as *mut u8);
                Ok(Variant::BigInt(BigInt::from_signed_bytes_le(b)))
            },
            ValueType::Bytes => {
                let len: u64 = unsafe { mem::transmute_copy(&num) };
                Ok(Variant::Bytes(unsafe {
                    Bytes::from_raw_parts(buf, len as usize)
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

extern "C" fn abi_path(class: *const AttrClass, len: *mut u64) -> *const u8 {
    unsafe {
        let path = &(*class).path;
        *len = path.len() as u64;
        path.as_ptr()
    }
}

extern "C" fn abi_is_match(class: *const AttrClass, id: Token) -> u8 {
    let class = unsafe { &(*class) };
    if class.id == id || class.aliases.iter().any(|&x| x == id) {
        1
    } else {
        0
    }
}

extern "C" fn abi_typ(class: *const AttrClass, len: *mut u64) -> *const u8 {
    unsafe {
        let typ = &(*class).typ;
        *len = typ.len() as u64;
        typ.as_ptr()
    }
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
    let slice = unsafe { Bytes::from_raw_parts(*data, len as usize) };

    match (cast)(attr, &slice) {
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
            Variant::BigInt(val) => {
                let val = val.to_signed_bytes_le();
                let len = val.len() as u64;
                unsafe {
                    *data = Box::into_raw(val.into_boxed_slice()) as *mut u8;
                    *(num as *mut u64) = len;
                };
                ValueType::BigInt
            }
            Variant::Buffer(val) => {
                let len = val.len() as u64;
                unsafe {
                    *data = Box::into_raw(val) as *mut u8;
                    *(num as *mut u64) = len as u64;
                };
                ValueType::Buffer
            }
            Variant::Bytes(val) => {
                unsafe {
                    *data = val.as_ptr();
                    *(num as *mut u64) = val.len() as u64;
                };
                ValueType::Bytes
            }
            _ => ValueType::Nil,
        },
        Err(e) => {
            unsafe { *err = SafeString::from(&format!("{}", e)) }
            ValueType::Error
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        attr::{Attr, AttrClass},
        bytes::{Bytes, TryGet},
        token::Token,
        variant::Variant,
    };
    use failure::err_msg;
    use std::str::from_utf8;

    #[test]
    fn bool() {
        #[derive(Clone)]
        struct TestCast {}

        let class = AttrClass::builder("bool")
            .typ("@bool")
            .byte_range(0..1)
            .cast(|_, data| Ok(Variant::Bool(data[0] == 1)))
            .build();
        let attr = Attr::new(&class, 0..8, Bytes::from(&[1][..]));
        assert_eq!(attr.id(), Token::from("bool"));
        assert_eq!(attr.typ(), "@bool");
        assert_eq!(attr.byte_range(), 0..1);

        match attr.get().unwrap() {
            Variant::Bool(val) => assert!(val),
            _ => panic!(),
        };
    }

    #[test]
    fn u64() {
        let class = AttrClass::builder("u64")
            .typ("@u64")
            .byte_range(0..6)
            .cast(|_, data| Ok(Variant::UInt64(from_utf8(data).unwrap().parse().unwrap())))
            .build();
        let attr = Attr::new(&class, 0..48, Bytes::from(&b"123456789"[..]));
        assert_eq!(attr.id(), Token::from("u64"));
        assert_eq!(attr.typ(), "@u64");
        assert_eq!(attr.byte_range(), 0..6);

        match attr.get().unwrap() {
            Variant::UInt64(val) => assert_eq!(val, 123_456_789),
            _ => panic!(),
        };
    }

    #[test]
    fn i64() {
        let class = AttrClass::builder("i64")
            .typ("@i64")
            .byte_range(0..6)
            .cast(|_, data| Ok(Variant::Int64(from_utf8(data).unwrap().parse().unwrap())))
            .build();
        let attr = Attr::new(&class, 0..48, Bytes::from(&b"-123456789"[..]));
        assert_eq!(attr.id(), Token::from("i64"));
        assert_eq!(attr.typ(), "@i64");
        assert_eq!(attr.byte_range(), 0..6);

        match attr.get().unwrap() {
            Variant::Int64(val) => assert_eq!(val, -123456789),
            _ => panic!(),
        };
    }

    #[test]
    fn buffer() {
        let class = AttrClass::builder("buffer")
            .typ("@buffer")
            .byte_range(0..6)
            .cast(|_, data| Ok(Variant::Buffer(data.to_vec().into_boxed_slice())))
            .build();
        let attr = Attr::new(&class, 0..48, Bytes::from(&b"123456789"[..]));
        assert_eq!(attr.id(), Token::from("buffer"));
        assert_eq!(attr.typ(), "@buffer");
        assert_eq!(attr.byte_range(), 0..6);

        match attr.get().unwrap() {
            Variant::Buffer(val) => assert_eq!(&*val, b"123456789"),
            _ => panic!(),
        };
    }

    #[test]
    fn slice() {
        let class = AttrClass::builder("slice")
            .typ("@slice")
            .byte_range(0..6)
            .cast(|_, data| data.get(0..3).map(Variant::Bytes))
            .build();
        let attr = Attr::new(&class, 0..48, Bytes::from(&b"123456789"[..]));
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), "@slice");
        assert_eq!(attr.byte_range(), 0..6);

        match attr.get().unwrap() {
            Variant::Bytes(val) => assert_eq!(val, Bytes::from(&b"123"[..])),
            _ => panic!(),
        };
    }

    #[test]
    fn error() {
        let class = AttrClass::builder("slice")
            .typ("@slice")
            .byte_range(0..6)
            .cast(|_, _| Err(err_msg("oh no!")))
            .build();
        let attr = Attr::new(&class, 0..48, Bytes::from(&b"123456789"[..]));
        assert_eq!(attr.id(), Token::from("slice"));
        assert_eq!(attr.typ(), "@slice");
        assert_eq!(attr.byte_range(), 0..6);

        match attr.get() {
            Err(err) => assert_eq!(format!("{}", err), "oh no!"),
            _ => panic!(),
        };
    }
}
