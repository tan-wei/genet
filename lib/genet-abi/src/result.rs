use std::{error::Error, result};

/// A result object.
pub type Result<T> = result::Result<T, Box<Error>>;
