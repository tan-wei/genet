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
//! - Address - Internal raw pointer

extern crate libc;

use std::fmt;
use std::mem;
use std::string::String;
use std::slice;
use super::symbol;

#[derive(Debug,PartialEq)]
pub enum Type {
    Nil = 0,
    Bool = 1,
    Int64 = 2,
    Uint64 = 3,
    Double = 4,
    String = 5,
    StringRef = 6,
    Slice = 7,
    Address = 12,
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
            Type::StringRef => write!(f, "Variant (StringRef)"),
            Type::Slice => write!(f, "Variant (Slice)"),
            Type::Address => write!(f, "Variant (Address)"),
        }
    }
}

pub trait Value<T> {
    fn get(&self) -> T;
    fn set(&mut self, &T);
}

pub trait ValueString {
    fn get(&self) -> String;
    fn set_copy(&mut self, &str);
    fn set_ref(&mut self, &'static str);
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
        self.set_nil();
        self.set_typ_tag(Type::Bool, 0);
        self.val.boolean = *val;
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
        self.set_nil();
        self.set_typ_tag(Type::Int64, 0);
        self.val.int64 = *val;
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
        self.set_nil();
        self.set_typ_tag(Type::Uint64, 0);
        self.val.uint64 = *val;
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
        self.set_nil();
        self.set_typ_tag(Type::Double, 0);
        self.val.double = *val;
    }
}

impl ValueString for Variant {
     fn get(&self) -> String {
        unsafe {
            let mut size: libc::size_t = 0;
            let slice = symbol::Variant_string.unwrap()(self, &mut size);
            String::from_utf8_unchecked(slice::from_raw_parts(slice, size as usize).to_vec())
        }
    }

    fn set_copy(&mut self, val: &str) {
        unsafe { symbol::Variant_setString.unwrap()(self, val.as_ptr() as *const i8, val.len()) }
    }

    fn set_ref(&mut self, val: &'static str) {
        unsafe { symbol::Variant_setStringRef.unwrap()(self, val.as_ptr() as *const i8, val.len()) }
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
        self.set_nil();
        self.set_typ_tag(Type::Slice, val.len() as u64);
        self.val.data = val.as_ptr();
    }
}

impl Variant {
    fn set_typ_tag(&mut self, typ: Type, tag: u64) {
        self.typ_tag = (tag << 4) | (typ as u8 & 0b1111) as u64
    }

    pub fn typ(&self) -> Type {
        unsafe { mem::transmute((self.typ_tag & 0b1111) as u8) }
    }

    pub fn tag(&self) -> u64 {
        (self.typ_tag >> 4) as u64
    }

    pub fn set_nil(&mut self) {
        match self.typ() {
            Type::String => unsafe { symbol::Variant_setNil.unwrap()(self) },
            Type::Nil => (),
            _ => self.set_typ_tag(Type::Nil, 0)
        }
    }
}