use genet_abi::env;
use genet_napi::napi::{Env, Result, Value};

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    let tk = env.create_object()?;
    env.set_named_property(
        tk,
        "abi",
        env.create_double(env::genet_abi_version() as f64)?,
    )?;
    env.set_named_property(exports, "version", tk)?;
    Ok(())
}
