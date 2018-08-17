extern crate libc;

#[macro_use]
extern crate lazy_static;

pub mod attr;
pub mod cast;
pub mod context;
pub mod decoder;
pub mod env;
pub mod error;
pub mod fixed;
pub mod io;
pub mod layer;
pub mod result;
pub mod slice;
pub mod token;
pub mod variant;

mod string;
mod vec;
