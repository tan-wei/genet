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
    Int32 = 2,
    Uint32 = 3,
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
    int32: i32,
    uint32: u32,
    slice: *mut (*const u8, *const u8),
    ptr: *mut (),
}

#[repr(C)]
pub struct Variant {
    typ_tag: u8,
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
            Type::Int32 => {
                let val: i32 = Value::get(self);
                write!(f, "Variant ({})", val)
            }
            Type::Uint32 => {
                let val: u32 = Value::get(self);
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
                Type::Int32 => self.val.int32 != 0,
                Type::Uint32 => self.val.uint32 != 0,
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
        Value::<i32>::get(self) as i8
    }

    fn set(&mut self, val: &i8) {
        Value::<i32>::set(self, &(*val as i32))
    }
}

impl Value<i16> for Variant {
    fn get(&self) -> i16 {
        Value::<i32>::get(self) as i16
    }

    fn set(&mut self, val: &i16) {
        Value::<i32>::set(self, &(*val as i32))
    }
}

impl Value<i32> for Variant {
    fn get(&self) -> i32 {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean as i32,
                Type::Int32 => self.val.int32 as i32,
                Type::Uint32 => self.val.uint32 as i32,
                Type::Double => self.val.double as i32,
                _ => 0
            }
        }
    }

    fn set(&mut self, val: &i32) {
        if self.is_fat() {
            unsafe { symbol::Variant_setInt32.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Int32);
            self.val.int32 = *val
        }
    }
}


impl Value<u8> for Variant {
    fn get(&self) -> u8 {
        Value::<i32>::get(self) as u8
    }

    fn set(&mut self, val: &u8) {
        Value::<i32>::set(self, &(*val as i32))
    }
}

impl Value<u16> for Variant {
    fn get(&self) -> u16 {
        Value::<i32>::get(self) as u16
    }

    fn set(&mut self, val: &u16) {
        Value::<i32>::set(self, &(*val as i32))
    }
}

impl Value<u32> for Variant {
    fn get(&self) -> u32 {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean as u32,
                Type::Int32 => self.val.int32 as u32,
                Type::Uint32 => self.val.uint32 as u32,
                Type::Double => self.val.double as u32,
                _ => 0
            }
        }
    }

    fn set(&mut self, val: &u32) {
        if self.is_fat() {
            unsafe { symbol::Variant_setUint32.unwrap()(self, *val) }
        } else {
            self.set_typ(Type::Uint32);
            self.val.uint32 = *val
        }
    }
}

impl Value<f32> for Variant {
    fn get(&self) -> f32 {
        Value::<f64>::get(self) as f32
    }

    fn set(&mut self, val: &f32) {
        Value::<f64>::set(self, &(*val as f64))
    }
}

impl Value<f64> for Variant {
    fn get(&self) -> f64 {
        unsafe {
            match self.typ() {
                Type::Bool => self.val.boolean as u8 as f64,
                Type::Int32 => self.val.int32 as f64,
                Type::Uint32 => self.val.uint32 as f64,
                Type::Double => self.val.double as f64,
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

impl Value<String> for Variant {
    fn get(&self) -> String {
        unsafe {
            let slice = CStr::from_ptr(symbol::Variant_string.unwrap()(self));
            String::from_utf8_unchecked(slice.to_bytes().to_vec())
        }
    }

    fn set(&mut self, val: &String) {
        unsafe { symbol::Variant_setString.unwrap()(self, val.as_str().as_ptr() as *const i8) }
    }
}

impl Value<&'static [u8]> for Variant {
    fn get(&self) -> &'static [u8] {
        unsafe {
            match self.typ() {
                Type::Slice => {
                    let (begin, end) = *self.val.slice;
                    slice::from_raw_parts(begin, (end as usize) - (begin as usize))
                },
                _ => &[]
            }
        }
    }

    fn set(&mut self, val: &&'static [u8]) {
        unsafe {
            let begin = val.as_ptr();
            let end = begin.offset(val.len() as isize);
            symbol::Variant_setSlice.unwrap()(self, (begin, end));
        }
    }
}

impl Variant {
    pub fn set_typ(&mut self, typ: Type) {
        self.typ_tag = (self.typ_tag & 0b1111_0000) | (typ as u8)
    }

    pub fn typ(&self) -> Type {
        unsafe { mem::transmute(self.typ_tag & 0b0000_1111) }
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
            Type::Nil | Type::Bool | Type::Int32 | Type::Uint32 | Type::Double => false,
            _ => true
        }
    }
}
