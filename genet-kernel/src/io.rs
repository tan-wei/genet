use frame::Frame;
use genet_abi::{fixed::MutFixed, layer::Layer, result::Result};
use std::fmt::Debug;

pub trait Output: Send + Debug {
    fn write(&mut self, frames: &[&Frame]) -> Result<()>;
    fn end(&mut self) -> Result<()>;
}

pub trait Input: Send + Debug {
    fn read(&mut self) -> Result<Vec<MutFixed<Layer>>>;
}
