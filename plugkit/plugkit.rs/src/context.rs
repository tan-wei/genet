//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::variant::Variant;
use super::layer::{Layer, Confidence};
use super::symbol;
use super::token::Token;
use std::io::{Error, ErrorKind};

#[repr(C)]
pub struct Context {
    close_stream: bool,
    confidence_threshold: u32
}

impl Context {
    pub fn get_config(&self, name: &str) -> &Variant {
        unsafe {
            &*symbol::Context_getConfig.unwrap()(self, name.as_ptr() as *const i8, name.len())
        }
    }

    pub fn close_stream(&mut self) {
        self.close_stream = true
    }

    pub fn assert_confidence(&self, confidence: Confidence) -> Result<(), Error> {
        if confidence as u32 >= self.confidence_threshold {
            Ok(())
        } else {
            Err(Error::new(ErrorKind::Other, "insufficient confidence"))
        }
    }

    pub fn add_layer_linkage(&mut self, token: Token, id: u64, layer: &mut Layer) {
        unsafe {
            symbol::Context_addLayerLinkage.unwrap()(self, token, id, layer);
        }
    }
}
