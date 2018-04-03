//! Worker traits.

use std::io;
use super::layer::{Confidence, Layer};
use super::context::Context;

pub trait Worker {
    fn examine(&mut self, _ctx: &mut Context, _layer: &Layer) -> Confidence {
        Confidence::Exact
    }

    fn analyze(&mut self, _ctx: &mut Context, _layer: &mut Layer) -> Result<(), io::Error> {
        Ok(())
    }
}
