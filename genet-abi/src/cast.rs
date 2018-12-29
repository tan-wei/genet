use crate::{
    attr::{Attr, AttrClass, AttrClassBuilder, AttrContext, AttrField, AttrXField},
    slice,
    variant::Variant,
};
use std::{convert::Into, io::Result};

/// Cast trait.
pub trait Cast: 'static + Send + Sync {
    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Variant>;
}

/// Typed cast trait.
pub trait Typed: Cast {
    type Output: Into<Variant>;
    fn cast(&self, _attr: &Attr, data: &slice::ByteSlice) -> Result<Self::Output>;
}

/// Mappable cast trait.
pub trait Map
where
    Self: Sized,
{
    fn map<I, R, F>(self, func: F) -> Mapped<Self, I, R, F>
    where
        Self: Typed<Output = I>,
        I: Into<Variant>,
        R: Into<Variant>,
        F: Fn(I) -> R + Clone,
    {
        Mapped { cast: self, func }
    }
}

impl<T, X> Map for T
where
    T: Typed<Output = X>,
    X: Into<Variant>,
{
}

#[derive(Clone)]
pub struct Mapped<T, I, R, F>
where
    T: Typed<Output = I>,
    I: Into<Variant>,
    R: Into<Variant>,
    F: Fn(I) -> R + Clone,
{
    cast: T,
    func: F,
}

impl<T, I, R, F> Typed for Mapped<T, I, R, F>
where
    T: Typed<Output = I> + Clone + 'static,
    I: Into<Variant> + Clone + 'static,
    R: Into<Variant> + Clone + 'static,
    F: Fn(I) -> R + Clone + Send + Sync + 'static,
{
    type Output = R;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Self::Output> {
        Typed::cast(&self.cast, attr, data).map(self.func.clone())
    }
}

impl<T, X> Cast for T
where
    T: Typed<Output = X> + Clone,
    X: Into<Variant>,
{
    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Variant> {
        Typed::cast(self, attr, data).map(|r| r.into())
    }
}

impl<I: 'static + Into<Variant> + Clone, V: Typed<Output = I> + Clone> AttrField for V {
    type I = I;

    fn class(
        &self,
        ctx: &AttrContext,
        bit_size: usize,
        filter: Option<fn(Self::I) -> Variant>,
    ) -> AttrClassBuilder {
        let mut b = AttrClass::builder(ctx.path.clone());
        if let Some(f) = filter {
            b = b.cast(&self.clone().map(f));
        } else {
            b = b.cast(self)
        }
        b.typ(ctx.typ.clone())
            .aliases(ctx.aliases.clone())
            .bit_range(0, ctx.bit_offset..(ctx.bit_offset + bit_size))
            .name(ctx.name)
            .description(ctx.description)
    }
}

#[derive(Default, Clone)]
pub struct Nil();

impl Typed for Nil {
    type Output = Variant;

    fn cast(&self, _attr: &Attr, _data: &slice::ByteSlice) -> Result<Variant> {
        Ok(Variant::Nil)
    }
}

impl<I: 'static + Into<Variant> + Clone, T: Typed<Output = I> + Clone + Default> AttrXField for T {
    type Builder = TypedBuilder<I, T>;
}

pub struct TypedBuilder<I: 'static + Into<Variant> + Clone, T: Default + Typed<Output = I>> {
    data: T,
    id: String,
    name: &'static str,
    desc: &'static str,
    aliases: Vec<String>,
    filter: fn(I) -> Variant,
}

impl<I: 'static + Into<Variant> + Clone, T: Typed<Output = I> + Clone + Default> Default
    for TypedBuilder<I, T>
{
    fn default() -> Self {
        Self {
            data: T::default(),
            id: String::default(),
            name: "",
            desc: "",
            aliases: Vec::new(),
            filter: |x| x.into(),
        }
    }
}

impl<I: 'static + Into<Variant> + Clone, T: Typed<Output = I> + Clone + Default>
    Into<AttrClassBuilder> for TypedBuilder<I, T>
{
    fn into(self) -> AttrClassBuilder {
        AttrClass::builder(&self.id)
            .cast(&self.data.map(self.filter))
            .aliases(self.aliases)
            .name(self.name)
            .description(self.desc)
    }
}
