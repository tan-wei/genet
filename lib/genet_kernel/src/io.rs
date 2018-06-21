use frame::Frame;
use genet_ffi::result::Result;
use genet_ffi::{layer::Layer, ptr::MutPtr};
use std::fmt::Debug;
use std::time::Duration;

pub trait Output: Send + Debug {
    fn write(&self, frames: Option<&[&Frame]>) -> Result<()>;
}

pub trait Input: Send + Debug {
    fn read(&self, timeout: Duration) -> Result<Vec<MutPtr<Layer>>>;
}
