//! This crate provides APIs for creating plugin pacakges for
//! [genet](https://genet.app/).

extern crate byteorder;
extern crate genet_abi;

pub mod decoder;
pub mod helper;
pub mod io;
pub mod token;

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
