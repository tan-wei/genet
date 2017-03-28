use libc;
use plugkit::{context::Context as PContext, token::Token};
use session::Context;
use std::ffi::{CStr, CString};
use std::str;

#[no_mangle]
pub extern "C" fn genet_token_get(id: *const libc::c_char) -> Token {
    unsafe {
        let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
        Token::get(id)
    }
}

#[no_mangle]
pub extern "C" fn genet_token_string(id: Token) -> *mut libc::c_char {
    unsafe { CString::new(Token::string(id)).unwrap().into_raw() }
}

#[no_mangle]
pub extern "C" fn genet_context_close_stream(context: *mut Context) {
    unsafe { (*context).close_stream() }
}

#[no_mangle]
pub extern "C" fn genet_context_free(context: *mut Context) {
    unsafe {
        if !context.is_null() {
            Box::from_raw(context);
        }
    }
}

#[no_mangle]
pub extern "C" fn genet_context_get_config(
    context: *const Context,
    key: *const libc::c_char,
) -> *mut libc::c_char {
    unsafe {
        let key = str::from_utf8_unchecked(CStr::from_ptr(key).to_bytes());
        let s = (*context)
            .get_config(key)
            .or_else(|| Some(String::new()))
            .unwrap();
        CString::new(s).unwrap().into_raw()
    }
}
