pub mod alloc;
pub mod attr;
pub mod bytes;
pub mod codable;
pub mod context;
pub mod decoder;
pub mod file;
pub mod filter;
pub mod fixed;
pub mod layer;
pub mod metadata;
pub mod package;
pub mod reader;
pub mod result;
pub mod token;
pub mod variant;
pub mod writer;

mod string;
mod vec;

use alloc::SharedAllocator;
use std::mem::size_of;

#[global_allocator]
static ALLOC: SharedAllocator = SharedAllocator;

#[no_mangle]
pub extern "C" fn genet_abi_signature(
    data: *const (),
    cb: extern "C" fn(*const u8, u64, *const ()),
) {
    let abi_signature = abi_signature();
    (cb)(abi_signature.as_ptr(), abi_signature.len() as u64, data);
}

pub fn abi_signature() -> String {
    format!(
        "abi={}.{} layer_size={}",
        env!("CARGO_PKG_VERSION_MAJOR").parse().unwrap_or(0),
        env!("CARGO_PKG_VERSION_MINOR").parse().unwrap_or(0),
        size_of::<layer::Layer>()
    )
}
