#[no_mangle]
pub extern "C" fn genet_abi_version() -> u64 {
    let major: u64 = env!("CARGO_PKG_VERSION_MAJOR").parse().unwrap_or(0);
    let minor: u64 = env!("CARGO_PKG_VERSION_MINOR").parse().unwrap_or(0);
    major << 32 | minor
}
