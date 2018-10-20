//! This crate provides APIs for creating plugin pacakges for
//! [genet](https://genet.app/).

extern crate byteorder;
extern crate genet_abi;
extern crate lazy_static;

pub mod decoder;
pub mod helper;
pub mod reader;
pub mod token;
pub mod writer;

pub mod attr;
pub mod cast;
pub mod context;
pub mod error;
pub mod fixed;
pub mod layer;
pub mod prelude;
pub mod result;
pub mod slice;
pub mod variant;

#[doc(hidden)]
pub use lazy_static::*;
