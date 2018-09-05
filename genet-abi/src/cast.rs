use slice::{self, TryGet};
use std::{
    convert::Into,
    io::Result,
    ops::{Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
};
use variant::Variant;

/// Cast trait.
pub trait Cast: Send + Sync + CastClone {
    fn cast(&self, &slice::ByteSlice) -> Result<Variant>;
}

pub trait CastClone {
    fn clone_box(&self) -> Box<Cast>;
    fn into_box(self) -> Box<Cast>;
}

impl<T> CastClone for T
where
    T: 'static + Cast + Clone,
{
    fn clone_box(&self) -> Box<Cast> {
        Box::new(self.clone())
    }

    fn into_box(self) -> Box<Cast> {
        Box::new(self)
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
    fn cast(&self, data: &slice::ByteSlice) -> Result<Self::Output>;
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

    fn cast(&self, data: &slice::ByteSlice) -> Result<Self::Output> {
        self.cast.cast(data).map(self.func.clone())
    }
}

impl<T, X> Cast for T
where
    T: 'static + Typed<Output = X> + Send + Sync + Clone,
    X: Into<Variant>,
{
    fn cast(&self, data: &slice::ByteSlice) -> Result<Variant> {
        T::cast(self, data).map(|r| r.into())
    }
}

/// Nil cast.
#[derive(Clone)]
pub struct Nil();

impl Cast for Nil {
    fn cast(&self, _data: &slice::ByteSlice) -> Result<Variant> {
        Ok(Variant::Nil)
    }
}

/// Constant value cast.
#[derive(Clone)]
pub struct Const<T>(pub T);

impl<T: Into<Variant> + Clone> Typed for Const<T> {
    type Output = T;

    fn cast(&self, _data: &slice::ByteSlice) -> Result<T> {
        Ok(self.0.clone())
    }
}

/// Ranged combinator.
#[derive(Clone)]
pub struct Ranged<T, R>(pub T, pub R);

macro_rules! impl_ranged {
    ( $( $x:ty ), * ) => {
        $(
            impl<T, X> Typed for Ranged<T, $x>
            where
                T: 'static + Typed<Output = X> + Clone,
                X: Into<Variant>,
            {
                type Output = X;

                fn cast(&self, data: &slice::ByteSlice) -> Result<Self::Output> {
                    self.0.cast(&data.try_get(self.1.clone())?)
                }
            }
        )*
    };
}

impl_ranged!(
    Range<usize>,
    RangeFrom<usize>,
    RangeFull,
    RangeInclusive<usize>,
    RangeTo<usize>,
    RangeToInclusive<usize>
);
