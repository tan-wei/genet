extern crate libc;

#[macro_use]
extern crate lazy_static;

pub mod attr;
pub mod context;
pub mod decoder;
pub mod dissector;
pub mod env;
pub mod error;
pub mod io;
pub mod layer;
pub mod fixed;
pub mod result;
pub mod slice;
pub mod token;
pub mod variant;

mod string;
mod vec;
