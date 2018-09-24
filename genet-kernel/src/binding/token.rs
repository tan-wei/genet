use genet_abi::token::Token;
use genet_napi::napi::{CallbackInfo, Env, Result, Status, Value};
use libc;
use std::{
    ffi::{CStr, CString},
    str,
};

fn token_get<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
    if let Some(id) = info.argv().get(0) {
        let id = env.get_value_string(id)?;
        env.create_uint32(Token::from(id.as_str()).into())
    } else {
        Err(Status::InvalidArg)
    }
}

fn token_string<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
    if let Some(id) = info.argv().get(0) {
        let id = env.get_value_uint32(id)?;
        env.create_string(&Token::from(id).to_string())
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
