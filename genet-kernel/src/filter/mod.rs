use frame::Frame;
use std::fmt::Debug;

pub mod ast;
pub mod parser;
pub mod variant;

pub trait Worker {
    fn test(&self, &Frame) -> bool {
        true
    }
}

pub trait Filter: Send + Debug {
    fn new_worker(&self) -> Box<Worker>;
}
