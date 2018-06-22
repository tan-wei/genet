use frame::Frame;
use genet_abi::result::Result;
use genet_abi::{layer::Layer, ptr::MutPtr};
use std::fmt::Debug;
use std::time::Duration;

pub trait Output: Send + Debug {
    fn write(&mut self, frames: Option<&[&Frame]>) -> Result<()>;
}

pub trait Input: Send + Debug {
    fn read(&mut self, timeout: Duration) -> Result<Vec<MutPtr<Layer>>>;
}
