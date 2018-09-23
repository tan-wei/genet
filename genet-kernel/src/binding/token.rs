use genet_abi::token::Token;
use genet_napi::napi::{CallbackInfo, Env, Result, Status, Value};
use libc;
use std::{
    ffi::{CStr, CString},
    str,
};

#[no_mangle]
pub unsafe extern "C" fn genet_token_get(id: *const libc::c_char) -> Token {
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    Token::from(id)
}

#[no_mangle]
pub unsafe extern "C" fn genet_token_string(id: Token) -> *mut libc::c_char {
    CString::new(id.to_string()).unwrap().into_raw()
}

fn token_get<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
    if let Some(id) = info.argv().get(0) {
        let id = env.get_value_string(id)?;
        let token: u64 = Token::from(id.as_str()).into();
        env.create_uint32(token as u32)
    } else {
        Err(Status::InvalidArg)
    }
}

fn token_string<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
    if let Some(id) = info.argv().get(0) {
        let id = env.get_value_uint32(id)?;
        let token = Token::from(id as u64);
        env.create_string(&token.to_string())
    } else {
        Err(Status::InvalidArg)
    }
}

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    let tk = env.create_object()?;
    env.set_named_property(tk, "get", env.create_function("get", token_get)?)?;
    env.set_named_property(tk, "string", env.create_function("string", token_string)?)?;
    env.set_named_property(exports, "Token", tk)?;
    Ok(())
}
