use genet_napi::napi::{Env, Value};
use libc::c_char;
use std::ffi::CString;

pub mod attr;
pub mod embed;
pub mod frame;
pub mod layer;
pub mod session;
pub mod token;
pub mod version;

#[no_mangle]
pub unsafe extern "C" fn genet_str_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        CString::from_raw(ptr);
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_napi_init(env: *mut Env, exports: *mut Value) -> *mut Value {
    let env = &mut *env;
    let exports = &mut *exports;
    let _ = version::init(env, exports);
    let _ = token::init(env, exports);
    exports
}
