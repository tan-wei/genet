pub mod alloc;
pub mod attr;
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
pub mod slice;
pub mod token;
pub mod variant;
pub mod writer;

mod string;
mod vec;

use alloc::SharedAllocator;

#[global_allocator]
static ALLOC: SharedAllocator = SharedAllocator;

#[no_mangle]
pub extern "C" fn genet_abi_version() -> u64 {
    let major: u64 = env!("CARGO_PKG_VERSION_MAJOR").parse().unwrap_or(0);
    let minor: u64 = env!("CARGO_PKG_VERSION_MINOR").parse().unwrap_or(0);
    major << 32 | minor
}
