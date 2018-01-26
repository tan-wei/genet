use std::io::{Error, ErrorKind};
use std::mem;
use super::token;
use super::token::Token;
use super::range::Range;
use super::variant::{Value, ValueArray, ValueMap, Variant};
use super::symbol;

lazy_static! {
    static ref ERROR_TOKEN: Token   = token::get("!error");
    static ref EOF_TOKEN: Token     = token::get("!out-of-bounds");
    static ref INVALID_TOKEN: Token = token::get("!invalid-value");
}

pub enum Attr {}

impl<T> Value<T> for Attr
where
    Variant: Value<T>,
{
    fn get(&self) -> T {
        Value::get(self.value())
    }

    fn set(&mut self, val: &T) {
        Value::set(self.value_mut(), val)
    }
}

impl<T> ValueArray<T> for Attr
where
    Variant: ValueArray<T>,
{
    fn get(&self, index: usize) -> T {
        ValueArray::get(self.value(), index)
    }

    fn set(&mut self, index: usize, val: &T) {
        ValueArray::set(self.value_mut(), index, val)
    }
}

impl<T> ValueMap<T> for Attr
where
    Variant: ValueMap<T>,
{
    fn get(&self, key: &str) -> T {
        ValueMap::get(self.value(), key)
    }

    fn set(&mut self, key: &str, val: &T) {
        ValueMap::set(self.value_mut(), key, val)
    }
}

pub trait ResultValue<T> {
    fn set_result(&mut self, res: Result<(T, Range), Error>) -> Result<(), Error>;
}

impl<T> ResultValue<T> for Attr
where
    Variant: Value<T>,
{
    fn set_result(&mut self, res: Result<(T, Range), Error>) -> Result<(), Error> {
        match { res } {
            Ok(r) => {
                let (val, range) = r;
                Value::set(self.value_mut(), &val);
                self.set_range(&range)
            }
            Err(e) => {
                let err = match e.kind() {
                    ErrorKind::InvalidInput => *INVALID_TOKEN,
                    ErrorKind::InvalidData => *INVALID_TOKEN,
                    ErrorKind::UnexpectedEof => *EOF_TOKEN,
                    _ => *ERROR_TOKEN,
                };
                self.set_error(err);
                return Err(e);
            }
        }
        Ok(())
    }
}

impl Attr {
    pub fn id(&self) -> Token {
        unsafe { symbol::Attr_id.unwrap()(self) }
    }

    pub fn range(&self) -> Range {
        unsafe {
            let (start, end) = symbol::Attr_range.unwrap()(self);
            Range {
                start: start,
                end: end,
            }
        }
    }

    pub fn set_range(&mut self, range: &Range) {
        unsafe { symbol::Attr_setRange.unwrap()(self, (range.start, range.end)) }
    }

    pub fn typ(&self) -> Token {
        unsafe { symbol::Attr_type.unwrap()(self) }
    }

    pub fn set_typ(&mut self, id: Token) {
        unsafe { symbol::Attr_setType.unwrap()(self, id) }
    }

    pub fn error(&self) -> Token {
        unsafe { symbol::Attr_error.unwrap()(self) }
    }

    pub fn set_error(&mut self, id: Token) {
        unsafe { symbol::Attr_setError.unwrap()(self, id) }
    }

    pub fn value(&self) -> &Variant {
        unsafe { mem::transmute(&*self as *const _) }
    }

    pub fn value_mut(&mut self) -> &mut Variant {
        unsafe { mem::transmute(&mut *self as *mut _) }
    }

    pub fn set_nil(&mut self) {
        self.value_mut().set_nil()
    }
}
