use genet_abi::token::Token;
use libc;
use std::{
    ffi::{CStr, CString},
    str,
};
use genet_napi::napi::{Env, Value, Result};

#[no_mangle]
pub unsafe extern "C" fn genet_token_get(id: *const libc::c_char) -> Token {
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    Token::from(id)
}

#[no_mangle]
pub unsafe extern "C" fn genet_token_string(id: Token) -> *mut libc::c_char {
    CString::new(id.to_string()).unwrap().into_raw()
}

pub fn init(env: &mut Env, exports: &mut Value) -> Result<()> {
    let tk = env.create_object()?;
    env.set_property(exports, env.create_string("Token")?, tk)?;
    Ok(())
}