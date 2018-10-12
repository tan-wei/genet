use genet_abi::env;
use genet_napi::napi::{Env, Result, Value};

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    let tk = env.create_object()?;
    let abi = env::genet_abi_version();
    let abi = format!("{}.{}", abi >> 32, abi & 0xffffffff);
    env.set_named_property(tk, "abi", env.create_string(&abi)?)?;
    env.set_named_property(exports, "version", tk)?;
    Ok(())
}
