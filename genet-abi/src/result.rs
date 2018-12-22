use failure::Error;
use std::result;

/// A result object.
pub type Result<T> = result::Result<T, Error>;
