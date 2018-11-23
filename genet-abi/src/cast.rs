use attr::{Attr, AttrClass, AttrContext, AttrField, AttrList, SizedAttrField};
use fixed::Fixed;
use num_traits::Num;
use slice;
use std::{convert::Into, io::Result, mem::size_of};
use variant::Variant;

/// Cast trait.
pub trait Cast: Send + Sync + CastClone {
    fn cast(&self, _attr: &Attr, &slice::ByteSlice) -> Result<Variant>;
}

pub trait CastClone {
    fn clone_box(&self) -> Box<Cast>;
}

impl<T> CastClone for T
where
    T: 'static + Cast + Clone,
{
    fn clone_box(&self) -> Box<Cast> {
        Box::new(self.clone())
    }
}

impl Clone for Box<Cast> {
    fn clone(&self) -> Box<Cast> {
        self.clone_box()
    }
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

impl<T: Into<Variant>, V: Typed<Output = T> + Cast> AttrField for V {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        AttrList {
            class: Fixed::new(
                AttrClass::builder(ctx.path.clone())
                    .cast(self)
                    .typ(ctx.typ.clone())
                    .name(ctx.name)
                    .description(ctx.description)
                    .build(),
            ),
            aliases: Vec::new(),
        }
    }
}

impl<T, X> SizedAttrField for T
where
    T: AttrField + Typed<Output = X>,
    X: Into<Variant> + Num,
{
    fn bit_size(&self) -> usize {
        size_of::<X>() * 8
    }
}
