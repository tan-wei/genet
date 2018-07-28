use frame::Frame;
use genet_abi::{layer::Layer, fixed::MutFixed, result::Result};
use std::fmt::Debug;

pub trait Output: Send + Debug {
    fn write(&mut self, frames: &[&Frame]) -> Result<()>;
}

pub trait Input: Send + Debug {
    fn read(&mut self) -> Result<Vec<MutFixed<Layer>>>;
}
