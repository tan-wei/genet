#![allow(non_upper_case_globals)]

extern crate libc;

use std;
use std::ffi::CString;
use super::variant::Variant;
use super::layer;
use super::layer::Layer;
use super::payload::Payload;
use super::attr::Attr;
use super::context::Context;
use super::token::Token;

macro_rules! def_func {
    ( $( $x:ident, $y:ty ); *; ) => {
        $(
        pub(crate) static mut $x: Option<$y> = None;
        )*
        pub fn init(resolve: fn(*const libc::c_char) -> *const ()) {
            unsafe {
                $(
                    $x = std::mem::transmute(
                        resolve(CString::new(stringify!($x)).unwrap().as_ptr()));
                )*
            }
        }
    };
}

def_func!(
    Token_literal_,        extern "C" fn(*const libc::c_char, libc::size_t) -> Token;
    Token_string,          extern "C" fn(Token) -> *const libc::c_char;
    Token_join,            extern "C" fn(Token, Token) -> Token;
    Context_getOption,     extern "C" fn(*const Context, *const libc::c_char) -> *const Variant;
    Variant_setNil,        extern "C" fn(*mut Variant);
    Variant_bool,          extern "C" fn(*const Variant) -> bool;
    Variant_setBool,       extern "C" fn(*mut Variant, bool);
    Variant_int32,         extern "C" fn(*const Variant) -> i32;
    Variant_setInt32,      extern "C" fn(*mut Variant, i32);
    Variant_uint32,        extern "C" fn(*const Variant) -> u32;
    Variant_setUint32,     extern "C" fn(*mut Variant, u32);
    Variant_double,        extern "C" fn(*const Variant) -> f64;
    Variant_setDouble,     extern "C" fn(*mut Variant, f64);
    Variant_string,        extern "C" fn(*const Variant) -> *const libc::c_char;
    Variant_setString,     extern "C" fn(*mut Variant, *const libc::c_char);
    Variant_slice,         extern "C" fn(*const Variant) -> (*const u8, *const u8);
    Variant_setSlice,      extern "C" fn(*mut Variant, (*const u8, *const u8));
    Variant_arrayValue,    extern "C" fn(*const Variant, libc::size_t) -> *const Variant;
    Variant_arrayValueRef, extern "C" fn(*mut Variant, libc::size_t) -> *mut Variant;
    Variant_mapValue,      extern "C" fn(*const Variant, *const libc::c_char) -> *const Variant;
    Variant_mapValueRef,   extern "C" fn(*mut Variant, *const libc::c_char) -> *mut Variant;
    Layer_id,              extern "C" fn(*const Layer) -> Token;
    Layer_range,           extern "C" fn(*const Layer) -> (libc::size_t, libc::size_t);
    Layer_setRange,        extern "C" fn(*mut Layer, (libc::size_t, libc::size_t));
    Layer_confidence,      extern "C" fn(*const Layer) -> layer::Confidence;
    Layer_setConfidence,   extern "C" fn(*mut Layer, layer::Confidence);
    Layer_worker,          extern "C" fn(*const Layer) -> u8;
    Layer_setWorker,       extern "C" fn(*mut Layer, u8);
    Layer_attr,            extern "C" fn(*const Layer, Token) -> *const Attr;
    Layer_payloads,        extern "C" fn(*const Layer, *mut libc::size_t) -> *const *const Payload;
    Layer_addLayer,        extern "C" fn(*mut Layer, *mut Context, Token) -> *mut Layer;
    Layer_addSubLayer,     extern "C" fn(*mut Layer, *mut Context, Token) -> *mut Layer;
    Layer_addAttr,         extern "C" fn(*mut Layer, *mut Context, Token) -> *mut Attr;
    Layer_addPayload,      extern "C" fn(*mut Layer, *mut Context) -> *mut Payload;
    Layer_addTag,          extern "C" fn(*mut Layer, Token);
    Payload_addSlice,      extern "C" fn(*mut Payload, (*const u8, *const u8));
    Payload_slices,        extern "C" fn(*const Payload, *mut libc::size_t) -> *const (*const u8, *const u8);
    Payload_addAttr,       extern "C" fn(*mut Payload, *mut Context, Token) -> *mut Attr;
    Payload_type,          extern "C" fn(*const Payload) -> Token;
    Payload_setType,       extern "C" fn(*mut Payload, Token);
    Payload_range,         extern "C" fn(*const Payload) -> (libc::size_t, libc::size_t);
    Payload_setRange,      extern "C" fn(*mut Payload, (libc::size_t, libc::size_t));
);

#[macro_export]
macro_rules! plugkit_module(
    ($body:expr) => (
        #[no_mangle]
        pub extern "C" fn plugkit_v1_init(resolve: fn(*const libc::c_char) -> *const ()) {
            plugkit::symbol::init(resolve);
            fn init() {
                $body
            }
            init();
        }
    )
);
