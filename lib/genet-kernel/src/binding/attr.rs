use genet_abi::{attr::Attr, layer::Layer, token::Token, variant::Variant};
use std::{ffi::CString, ptr};

#[repr(i8)]
pub enum ValueType {
    Error = -1,
    Nil = 0,
    Bool = 1,
    Int64 = 2,
    UInt64 = 3,
    Float64 = 4,
    String = 5,
    Buffer = 6,
    ByteSlice = 7,
}

#[repr(C)]
pub union VarValue {
    uint64: u64,
    int64: i64,
    float64: f64,
}

#[repr(C)]
pub struct Var {
    typ: ValueType,
    value: VarValue,
    data: *const u8,
}

#[no_mangle]
pub unsafe extern "C" fn genet_attr_id(attr: *const Attr) -> Token {
    (*attr).id()
}

#[no_mangle]
pub unsafe extern "C" fn genet_attr_type(attr: *const Attr) -> Token {
    (*attr).typ()
}

#[no_mangle]
pub unsafe extern "C" fn genet_attr_range(attr: *const Attr, start: *mut u64, end: *mut u64) {
    let range = (*attr).range();
    *start = range.start as u64;
    *end = range.end as u64;
}

#[no_mangle]
pub unsafe extern "C" fn genet_attr_get(attr: *const Attr, layer: *const Layer) -> Var {
    let var = (*attr).try_get(&*layer);
    let mut typ = ValueType::Nil;
    let mut data: *const u8 = ptr::null();
    let mut value = VarValue { uint64: 0 };

    match var {
        Err(err) => {
            let s = err.description();
            typ = ValueType::Error;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = CString::new(s).unwrap().into_raw() as *const u8;
        }
        Ok(Variant::Bool(b)) => {
            typ = ValueType::Bool;
            value = VarValue {
                uint64: if b { 1 } else { 0 },
            };
        }
        Ok(Variant::Int64(v)) => {
            typ = ValueType::Int64;
            value = VarValue { int64: v };
        }
        Ok(Variant::UInt64(v)) => {
            typ = ValueType::UInt64;
            value = VarValue { uint64: v };
        }
        Ok(Variant::Float64(v)) => {
            typ = ValueType::Float64;
            value = VarValue { float64: v };
        }
        Ok(Variant::String(s)) => {
            typ = ValueType::String;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = CString::from_vec_unchecked(s.as_bytes().to_vec()).into_raw() as *const u8;
        }
        Ok(Variant::Buffer(s)) => {
            typ = ValueType::Buffer;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = CString::from_vec_unchecked(s.to_vec()).into_raw() as *const u8;
        }
        Ok(Variant::ByteSlice(s)) => {
            typ = ValueType::ByteSlice;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = s.as_ptr();
        }
        _ => (),
    }
    Var { typ, value, data }
}
