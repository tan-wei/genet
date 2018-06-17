use slice;
use std::convert::Into;
use std::io::Result;
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
