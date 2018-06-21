use frame::Frame;
use genet_ffi::result::Result;
use genet_ffi::{layer::Layer, ptr::MutPtr};
use std::fmt::Debug;

pub trait Output: Send + Debug {
    fn write(&self, frames: Option<&[&Frame]>) -> Result<()>;
}

pub trait InputCallback: Send {
    fn write(&self, result: Result<Vec<MutPtr<Layer>>>) -> bool;
}

pub trait Input: Send + Debug {
    fn start(&self, callback: Box<InputCallback>);
}
