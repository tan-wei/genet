use super::token::Token;
use super::range::Range;
use super::variant::{Variant,Value,ValueArray,ValueMap};
use super::symbol;

pub enum Attr {}

impl<T> Value<T> for Attr where Variant: Value<T> {
    fn get(&self) -> T {
        Value::get(self.value())
    }

    fn set(&mut self, val: T) {
        Value::set(self.value_mut(), val)
    }
}

impl<T> ValueArray<T> for Attr where Variant: ValueArray<T> {
    fn get(&self, index: usize) -> T {
        ValueArray::get(self.value(), index)
    }

    fn set(&mut self, index: usize, val: T) {
        ValueArray::set(self.value_mut(), index, val)
    }
}

impl<T> ValueMap<T> for Attr where Variant: ValueMap<T> {
    fn get(&self, key: &str) -> T {
        ValueMap::get(self.value(), key)
    }

    fn set(&mut self, key: &str, val: T) {
        ValueMap::set(self.value_mut(), key, val)
    }
}

impl Attr {
    pub fn id(&self) -> Token {
        unsafe {
            symbol::Attr_id.unwrap()(self)
        }
    }

    pub fn range(&self) -> Range {
        unsafe {
            let (start, end) = symbol::Attr_range.unwrap()(self);
            Range { start: start, end: end }
        }
    }

    pub fn set_range(&mut self, range: Range) {
        unsafe {
            symbol::Attr_setRange.unwrap()(self, (range.start, range.end))
        }
    }

    pub fn typ(&self) -> Token {
        unsafe {
            symbol::Attr_type.unwrap()(self)
        }
    }

    pub fn set_typ(&mut self, id: Token) {
        unsafe {
            symbol::Attr_setType.unwrap()(self, id)
        }
    }

    pub fn error(&self) -> Token {
        unsafe {
            symbol::Attr_error.unwrap()(self)
        }
    }

    pub fn set_error(&mut self, id: Token) {
        unsafe {
            symbol::Attr_setError.unwrap()(self, id)
        }
    }

    pub fn value(&self) -> &Variant {
        unsafe {
            &*symbol::Attr_value.unwrap()(self)
        }
    }

    pub fn value_mut(&mut self) -> &mut Variant {
        unsafe {
            &mut *symbol::Attr_valueRef.unwrap()(self)
        }
    }

    pub fn set_nil(&mut self) {
        self.value_mut().set_nil()
    }
}
