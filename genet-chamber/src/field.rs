use crate::{result::Result, variant::Variant};
use failure::err_msg;

pub trait Field {
    type Input: Into<Variant>;
    type Output: Into<Variant>;

    fn bind(&self) -> BoundField<Self> {
        BoundField { field: self }
    }
}

pub struct BoundField<'a, F: ?Sized + Field> {
    field: &'a F,
}

impl<'a, F: Field> BoundField<'a, F> {
    fn get() -> Result<F::Output> {
        Err(err_msg(""))
    }
}
