use crate::{
    attr::{
        Attr, AttrClass, AttrClassBuilder, AttrContext, AttrField, MetadataOption, NodeBuilder,
    },
    context::Context,
    slice,
    variant::Variant,
};
use std::{convert::Into, io::Result};

/// Cast trait.
pub trait Cast: Send + Sync {
    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Variant>;
}

/// Typed cast trait.
pub trait Typed {
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
    T: Typed<Output = I>,
    I: Into<Variant>,
    R: Into<Variant>,
    F: Fn(I) -> R + Clone,
{
    type Output = R;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Self::Output> {
        self.cast.cast(attr, data).map(self.func.clone())
    }
}

impl<T, X> Cast for T
where
    T: 'static + Typed<Output = X> + Send + Sync + Clone,
    X: Into<Variant>,
{
    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Variant> {
        T::cast(self, attr, data).map(|r| r.into())
    }
}

impl<I: 'static + Into<Variant> + Clone, V: 'static + Typed<Output = I> + Sync + Clone + Cast>
    AttrField for V
{
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

pub struct NilBuilder {}

impl MetadataOption for NilBuilder {}

impl Into<Nil> for NilBuilder {
    fn into(self) -> Nil {
        Nil()
    }
}

impl NodeBuilder<Self> for Nil {
    type Builder = NilBuilder;

    fn build(_ctx: &Context) -> NilBuilder {
        NilBuilder {}
    }
}
