use genet_abi::genet_abi_version;
use genet_napi::napi::{Env, Result, Value};

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    let tk = env.create_object()?;
    let abi = genet_abi_version();
    let abi = format!("{}.{}", abi >> 32, abi & 0xffff_ffff);
    env.set_named_property(tk, "abi", env.create_string(&abi)?)?;
    env.set_named_property(exports, "version", tk)?;
    Ok(())
}
