
use genet_abi::env;
use genet_abi::token::Token;
use std::ffi::{CStr, CString};
use libc;
use std::str;

#[no_mangle]
pub extern "C" fn genet_token_get(id: *const libc::c_char) -> Token {
    unsafe {
        let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
        env::token(id)
    }
}

#[no_mangle]
pub extern "C" fn genet_token_string(id: Token) -> *mut libc::c_char {
    unsafe { CString::new(env::string(id)).unwrap().into_raw() }
}