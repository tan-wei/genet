use std::fmt;
use std::string::String;
use std::slice;
use std::ffi::CStr;
use super::symbol;

pub enum Variant {}

#[derive(Debug)]
pub enum Type {
  Nil    = 0,
  Bool   = 1,
  Int32  = 2,
  Uint32 = 3,
  Double = 6,
  String = 7,
  Slice  = 8,
  Array  = 9,
  Map    = 10
}

impl fmt::Display for Variant {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.typ() {
            Type::Nil    => write!(f, "Variant (Nil)"),
            Type::Bool   => {
                let val: bool = Value::get(self);
                write!(f, "Variant ({})", val)
            },
            Type::Int32  => {
                let val: i32 = Value::get(self);
                write!(f, "Variant ({})", val)
            },
            Type::Uint32 => {
                let val: u32 = Value::get(self);
                write!(f, "Variant ({})", val)
            },
            Type::Double => {
                let val: f64 = Value::get(self);
                write!(f, "Variant ({})", val)
            },
            Type::String => write!(f, "Variant (String)"),
            Type::Slice  => write!(f, "Variant (Slice)"),
            Type::Array  => write!(f, "Variant (Array)"),
            Type::Map    => write!(f, "Variant (Map)"),
        }
    }
}

pub trait Value<T> {
  fn get(&self) -> T;
  fn set(&mut self, T);
}

pub trait ValueMap<T> {
  fn get(&self, &str) -> T;
  fn set(&mut self, &str, T);
}

pub trait ValueArray<T> {
  fn get(&self, usize) -> T;
  fn set(&mut self, usize, T);
}

impl<T> ValueArray<T> for Variant where Variant: Value<T> {
    fn get(&self, index: usize) -> T {
        unsafe {
            let var = &*symbol::Variant_arrayValue.unwrap()(self, index);
            let val : T = Value::get(var);
            val
        }
    }

    fn set(&mut self, index: usize, val: T) {
        unsafe {
            let var = &mut *symbol::Variant_arrayValueRef.unwrap()(self, index);
            Value::set(var, val);
        }
    }
}

impl<T> ValueMap<T> for Variant where Variant: Value<T> {
    fn get(&self, key: &str) -> T {
        unsafe {
            let c = key.as_ptr() as *const i8;
            let var = &*symbol::Variant_mapValue.unwrap()(self, c);
            let val : T = Value::get(var);
            val
        }
    }

    fn set(&mut self, key: &str, val: T) {
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
            symbol::Variant_bool.unwrap()(self)
        }
    }

    fn set(&mut self, val: bool) {
        unsafe {
            symbol::Variant_setBool.unwrap()(self, val)
        }
    }
}

impl Value<i32> for Variant {
    fn get(&self) -> i32 {
        unsafe {
            symbol::Variant_int32.unwrap()(self)
        }
    }

    fn set(&mut self, val: i32) {
        unsafe {
            symbol::Variant_setInt32.unwrap()(self, val)
        }
    }
}

impl Value<u32> for Variant {
    fn get(&self) -> u32 {
        unsafe {
            symbol::Variant_uint32.unwrap()(self)
        }
    }

    fn set(&mut self, val: u32) {
        unsafe {
            symbol::Variant_setUint32.unwrap()(self, val)
        }
    }
}

impl Value<f64> for Variant {
    fn get(&self) -> f64 {
        unsafe {
            symbol::Variant_double.unwrap()(self)
        }
    }

    fn set(&mut self, val: f64) {
        unsafe {
            symbol::Variant_setDouble.unwrap()(self, val)
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

    fn set(&mut self, val: String) {
        unsafe {
            symbol::Variant_setString.unwrap()(self, val.as_str().as_ptr() as *const i8)
        }
    }
}

impl Value<&'static[u8]> for Variant {
    fn get(&self) -> &'static[u8] {
        unsafe {
            let (begin, end) = symbol::Variant_slice.unwrap()(self);
            slice::from_raw_parts(begin, (end as usize) - (begin as usize))
        }
    }

    fn set(&mut self, val: &'static[u8]) {
        unsafe {
            let begin = val.as_ptr();
            let end = begin.offset(val.len() as isize);
            symbol::Variant_setSlice.unwrap()(self, (begin, end));
        }
    }
}

impl Variant {
    pub fn typ(& self) -> Type {
        unsafe {
            symbol::Variant_type.unwrap()(self)
        }
    }

    pub fn set_nil(&mut self) {
        unsafe {
            symbol::Variant_setNil.unwrap()(self)
        }
    }
}
