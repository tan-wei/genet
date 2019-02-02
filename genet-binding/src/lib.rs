use genet_napi::napi::{Env, Value};
use std::{ffi::CString, os::raw::c_char};

mod attr;
mod frame;
mod layer;
mod session;
mod version;

#[no_mangle]
pub unsafe extern "C" fn genet_str_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        CString::from_raw(ptr);
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_napi_init(env: *const Env, exports: *const Value) -> *const Value {
    let env = &*env;
    let exports = &*exports;
    let _ = version::init(env, exports);
    let _ = session::init(env, exports);
    env.set_constructor(JsClass::Frame as usize, &frame::wrapper(env));
    env.set_constructor(JsClass::Layer as usize, &layer::wrapper(env));
    env.set_constructor(JsClass::Attr as usize, &attr::wrapper(env));
    exports
}

pub enum JsClass {
    SessionProfile = 0,
    Frame = 1,
    Layer = 2,
    Attr = 3,
}
