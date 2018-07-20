use slice::{self, TryGet};
use std::{
    convert::Into,
    io::Result,
    ops::{Range, RangeFrom, RangeFull, RangeInclusive, RangeTo, RangeToInclusive},
};
use variant::Variant;

pub trait Decoder: Send + Sync + DecoderClone {
    fn decode(&self, &slice::Slice) -> Result<Variant>;
}

pub trait DecoderClone {
    fn clone_box(&self) -> Box<Decoder>;
}

impl<T> DecoderClone for T
where
    T: 'static + Decoder + Clone,
{
    fn clone_box(&self) -> Box<Decoder> {
        Box::new(self.clone())
    }
}

impl Clone for Box<Decoder> {
    fn clone(&self) -> Box<Decoder> {
        self.clone_box()
    }
}

pub trait Typed {
    type Output: Into<Variant>;
    fn decode(&self, data: &slice::Slice) -> Result<Self::Output>;
}

pub trait Map
where
    Self: Sized,
{
    fn map<I, R>(self, func: fn(I) -> R) -> Mapped<Self, I, R>
    where
        Self: Typed<Output = I>,
        I: Into<Variant>,
        R: Into<Variant>,
    {
        Mapped {
            decoder: self,
            func,
        }
    }
}

impl<T, X> Map for T
where
    T: Typed<Output = X>,
    X: Into<Variant>,
{
}

#[derive(Clone)]
pub struct Mapped<T, I, R>
where
    T: Typed<Output = I>,
    I: Into<Variant>,
    R: Into<Variant>,
{
    decoder: T,
    func: fn(data: I) -> R,
}

impl<T, I, R> Typed for Mapped<T, I, R>
where
    T: Typed<Output = I>,
    I: Into<Variant>,
    R: Into<Variant>,
{
    type Output = R;

    fn decode(&self, data: &slice::Slice) -> Result<Self::Output> {
        self.decoder.decode(data).map(self.func)
    }
}

impl<T, X> Decoder for T
where
    T: 'static + Typed<Output = X> + Send + Sync + Clone,
    X: Into<Variant>,
{
    fn decode(&self, data: &slice::Slice) -> Result<Variant> {
        T::decode(self, data).map(|r| r.into())
    }
}

#[derive(Clone)]
pub struct Nil();

impl Decoder for Nil {
    fn decode(&self, _data: &slice::Slice) -> Result<Variant> {
        Ok(Variant::Nil)
    }
}

#[derive(Clone)]
pub struct Const<T>(pub T);

impl<T: Into<Variant> + Clone> Typed for Const<T> {
    type Output = T;

    fn decode(&self, _data: &slice::Slice) -> Result<T> {
        Ok(self.0.clone())
    }
}

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

                fn decode(&self, data: &slice::Slice) -> Result<Self::Output> {
                    self.0.decode(&data.try_get(self.1.clone())?)
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
