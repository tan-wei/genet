use genet_abi::abi_signature;
use genet_napi::napi::{Env, Result, Value};

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    let tk = env.create_object()?;
    env.set_named_property(tk, "abi", env.create_string(&abi_signature())?)?;
    env.set_named_property(exports, "version", tk)?;
    Ok(())
}
