//! Dynamic-typed values.
//!
//! Type Variant represents a dynamic-typed value.
//! Variant can contain one of these types:
//!
//! - Nil
//! - Bool - [`bool`](https://doc.rust-lang.org/std/primitive.bool.html)
//! - Int64 - [`i64`](https://doc.rust-lang.org/std/primitive.i64.html)
//! - Uint64 - [`u64`](https://doc.rust-lang.org/std/primitive.u64.html)
//! - Double - [`f64`](https://doc.rust-lang.org/std/primitive.f64.html)
//! - String - [`String`](https://doc.rust-lang.org/std/string/struct.String.html)
//! - Slice - `&'static[u8]`
//! - Array - An array of `Variant`s.
//! - Map - A map of key `String`s and `Variant`s

use std::fmt;
use std::mem;
use std::string::String;
use std::slice;
use std::ffi::CStr;
use super::symbol;

#[derive(Debug)]
pub enum Type {
    Nil = 0,
    Bool = 1,
    Int64 = 2,
    Uint64 = 3,
    Double = 6,
    String = 7,
    Slice = 8,
    Array = 9,
    Map = 10,
}

#[repr(C)]
union ValueUnion {
    boolean: bool,
    double: f64,
    int64: i64,
    uint64: u64,
    data: *const u8,
    ptr: *mut (),
}

#[repr(C)]
pub struct Variant {
    typ_tag: u64,
    val: ValueUnion
}

impl fmt::Display for Variant {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.typ() {
            Type::Nil => write!(f, "Variant (Nil)"),
            Type::Bool => {
                let val: bool = Value::get(self);
                write!(f, "Variant ({})", val)
            }
            Type::Int64 => {
                let val: i64 = Value::get(self);
                write!(f, "Variant ({})", val)
            }
            Type::Uint64 => {
                let val: u64 = Value::get(self);
                write!(f, "Variant ({})", val)
            }
            Type::Double => {
                let val: f64 = Value::get(self);
                write!(f, "Variant ({})", val)
            }
            Type::String => write!(f, "Variant (String)"),
            Type::Slice => write!(f, "Variant (Slice)"),
            Type::Array => write!(f, "Variant (Array)"),
            Type::Map => write!(f, "Variant (Map)"),
        }
    }
}

pub trait Value<T> {
    fn get(&self) -> T;
    fn set(&mut self, &T);
}

pub trait ValueString {
    fn get(&self) -> String;
    fn set(&mut self, &str);
}

pub trait ValueMap<T> {
    fn get(&self, &str) -> T;
    fn set(&mut self, &str, &T);
}

pub trait ValueArray<T> {
    fn get(&self, usize) -> T;
    fn set(&mut self, usize, &T);
}

impl<T> ValueArray<T> for Variant
where
    Variant: Value<T>,
{
    fn get(&self, index: usize) -> T {
        unsafe {
            let var = &*symbol::Variant_arrayValue.unwrap()(self, index);
            let val: T = Value::get(var);
            val
        }
    }

    fn set(&mut self, index: usize, val: &T) {
        unsafe {
            let var = &mut *symbol::Variant_arrayValueRef.unwrap()(self, index);
            Value::set(var, val);
        }
    }
}

impl<T> ValueMap<T> for Variant
where
    Variant: Value<T>,
{
    fn get(&self, key: &str) -> T {
        unsafe {
            let c = key.as_ptr() as *const i8;
            let var = &*symbol::Variant_mapValue.unwrap()(self, c);
            let val: T = Value::get(var);
            val
        }
    }

    fn set(&mut self, key: &str, val: &T) {
        unsafe {
            let c = key.as_ptr() as *const i8;
            let var = &mut *symbol::Variant_mapValueRef.unwrap()(self, c);
            Value::set(var, val);
        }
    }
}

impl Value<bool> for Variant {
    fn get(&self) -> bool {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean,
                Type::Int64 => self.val.int64 != 0,
                Type::Uint64 => self.val.uint64 != 0,
                Type::Double => self.val.double != 0.0,
                _ => false
            }
        }
    }

    fn set(&mut self, val: &bool) {
        if self.is_fat() {
            unsafe { symbol::Variant_setBool.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Bool);
            self.val.boolean = *val
        }
    }
}

impl Value<i8> for Variant {
    fn get(&self) -> i8 {
        Value::<i64>::get(self) as i8
    }

    fn set(&mut self, val: &i8) {
        Value::<i64>::set(self, &i64::from(*val))
    }
}

impl Value<i16> for Variant {
    fn get(&self) -> i16 {
        Value::<i64>::get(self) as i16
    }

    fn set(&mut self, val: &i16) {
        Value::<i64>::set(self, &i64::from(*val))
    }
}

impl Value<i32> for Variant {
    fn get(&self) -> i32 {
        Value::<i64>::get(self) as i32
    }

    fn set(&mut self, val: &i32) {
        Value::<i64>::set(self, &i64::from(*val))
    }
}

impl Value<i64> for Variant {
    fn get(&self) -> i64 {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean as i64,
                Type::Int64 => self.val.int64 as i64,
                Type::Uint64 => self.val.uint64 as i64,
                Type::Double => self.val.double as i64,
                _ => 0
            }
        }
    }

    fn set(&mut self, val: &i64) {
        if self.is_fat() {
            unsafe { symbol::Variant_setInt64.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Int64);
            self.val.int64 = *val
        }
    }
}

impl Value<u8> for Variant {
    fn get(&self) -> u8 {
        Value::<u64>::get(self) as u8
    }

    fn set(&mut self, val: &u8) {
        Value::<u64>::set(self, &u64::from(*val))
    }
}

impl Value<u16> for Variant {
    fn get(&self) -> u16 {
        Value::<u64>::get(self) as u16
    }

    fn set(&mut self, val: &u16) {
        Value::<u64>::set(self, &u64::from(*val))
    }
}

impl Value<u32> for Variant {
    fn get(&self) -> u32 {
        Value::<u64>::get(self) as u32
    }

    fn set(&mut self, val: &u32) {
        Value::<u64>::set(self, &u64::from(*val))
    }
}

impl Value<u64> for Variant {
    fn get(&self) -> u64 {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean as u64,
                Type::Int64 => self.val.int64 as u64,
                Type::Uint64 => self.val.uint64 as u64,
                Type::Double => self.val.double as u64,
                _ => 0
            }
        }
    }

    fn set(&mut self, val: &u64) {
        if self.is_fat() {
            unsafe { symbol::Variant_setUint64.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Uint64);
            self.val.uint64 = *val
        }
    }
}

impl Value<f32> for Variant {
    fn get(&self) -> f32 {
        Value::<f64>::get(self) as f32
    }

    fn set(&mut self, val: &f32) {
        Value::<f64>::set(self, &(f64::from(*val)))
    }
}

impl Value<f64> for Variant {
    fn get(&self) -> f64 {
        unsafe {
            match self.typ() {
                Type::Bool => f64::from(self.val.boolean as u8),
                Type::Int64 => self.val.int64 as f64,
                Type::Uint64 => self.val.uint64 as f64,
                Type::Double => self.val.double,
                _ => 0.0
            }
        }
    }

    fn set(&mut self, val: &f64) {
        if self.is_fat() {
            unsafe { symbol::Variant_setDouble.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Double);
            self.val.double = *val;
        }
    }
}

impl ValueString for Variant {
     fn get(&self) -> String {
        unsafe {
            let slice = CStr::from_ptr(symbol::Variant_string.unwrap()(self));
            String::from_utf8_unchecked(slice.to_bytes().to_vec())
        }
    }

    fn set(&mut self, val: &str) {
        unsafe { symbol::Variant_setString.unwrap()(self, val.as_ptr() as *const i8) }
    }
}

impl Value<&'static [u8]> for Variant {
    fn get(&self) -> &'static [u8] {
        unsafe {
            match self.typ() {
                Type::Slice => {
                    slice::from_raw_parts(self.val.data, self.tag() as usize)
                },
                _ => &[]
            }
        }
    }

    fn set(&mut self, val: &&'static [u8]) {
        unsafe {
            symbol::Variant_setSlice.unwrap()(self, (val.as_ptr(), val.len()));
        }
    }
}

impl Variant {
    fn set_typ(&mut self, typ: Type) {
        self.typ_tag = self.typ_tag | (typ as u8 & 0b1111) as u64
    }

    pub fn typ(&self) -> Type {
        unsafe { mem::transmute((self.typ_tag & 0b1111) as u8) }
    }

    pub fn tag(&self) -> u64 {
        (self.typ_tag >> 4) as u64
    }

    pub fn set_nil(&mut self) {
        if self.is_fat() {
            unsafe { symbol::Variant_setNil.unwrap()(self) }
        } else {
            self.set_typ(Type::Nil);
        }
    }

    fn is_fat(&self) -> bool {
        match self.typ() {
            Type::Nil | Type::Bool | Type::Int64 | Type::Uint64 | Type::Double => false,
            _ => true
        }
    }
}
