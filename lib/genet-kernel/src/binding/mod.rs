use libc::c_char;
use std::ffi::CString;

pub mod session;

#[no_mangle]
pub extern "C" fn genet_str_free(ptr: *mut c_char) {
    unsafe {
        if !ptr.is_null() {
            CString::from_raw(ptr);
        }
    }
}
