//! Worker traits.

use std::io;
use super::layer::Layer;
use super::context::Context;

pub trait Worker {
    fn analyze(&mut self, _ctx: &mut Context, _layer: &mut Layer) -> Result<(), io::Error> {
        Ok(())
    }
}
