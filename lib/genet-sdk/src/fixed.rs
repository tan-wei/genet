//! Fixed-lifetime shareable containers.
//!
//! `Fixed` containers are dynamically allocated during a program execution but live forever.
//!
//! Don't create `Fixed` of a temporary object, or it will cause a serious memory leak.

pub use genet_abi::fixed::{Fixed, MutFixed};
