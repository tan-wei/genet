//! Dynamic-typed values.
//!
//! Type Variant represents a dynamic-typed value.
//! Variant can contain one of these types:
//!
//! - Nil
//! - Bool - [`bool`](https://doc.rust-lang.org/std/primitive.bool.html)
//! - Int64 - [`i64`](https://doc.rust-lang.org/std/primitive.i64.html)
//! - Uint64 - [`u64`](https://doc.rust-lang.org/std/primitive.u64.html)
//! - Double - [`f64`](https://doc.rust-lang.org/std/primitive.f64.html)
//! - String - `Box<str>`,
//! - Buffer - `Box<[u8]>`,
//! - Slice - `ByteArray`

pub use genet_abi::variant::{TryInto, Variant};
