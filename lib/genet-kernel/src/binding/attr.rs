use genet_abi::{attr::Attr, layer::Layer, token::Token, variant::Variant};
use std::ffi::CString;
use std::mem;
use std::ptr;

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
    Slice = 7,
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
pub extern "C" fn genet_attr_id(attr: *const Attr) -> Token {
    unsafe { (*attr).id() }
}

#[no_mangle]
pub extern "C" fn genet_attr_type(attr: *const Attr) -> Token {
    unsafe { (*attr).typ() }
}

#[no_mangle]
pub extern "C" fn genet_attr_get(attr: *const Attr, layer: *const Layer) -> Var {
    let var = unsafe { (*attr).get(&*layer) };
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
            data = unsafe { mem::transmute(CString::new(s).unwrap().into_raw()) };
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
            data = unsafe {
                mem::transmute(CString::from_vec_unchecked(s.as_bytes().to_vec()).into_raw())
            };
        }
        Ok(Variant::Buffer(s)) => {
            typ = ValueType::Buffer;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = unsafe { mem::transmute(CString::from_vec_unchecked(s.to_vec()).into_raw()) };
        }
        Ok(Variant::Slice(s)) => {
            typ = ValueType::Slice;
            value = VarValue {
                uint64: s.len() as u64,
            };
            data = s.as_ptr();
        }
        _ => (),
    }
    Var { typ, value, data }
}
