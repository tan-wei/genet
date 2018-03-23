//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::variant::Variant;
use super::layer::{Confidence, Layer};
use super::symbol;
use super::token::Token;
use std::io::{Error, ErrorKind};

/// A Context object.
#[repr(C)]
pub struct Context {
    close_stream: bool,
    confidence_threshold: u32,
}

impl Context {
    /// Returns a config value in the current profile.
    pub fn get_config(&self, name: &str) -> &Variant {
        unsafe {
            &*symbol::Context_getConfig.unwrap()(self, name.as_ptr() as *const i8, name.len())
        }
    }

    /// Closes the current stream session.
    ///
    /// This method only works in the stream dissector.
    pub fn close_stream(&mut self) {
        self.close_stream = true
    }

    /// Checks the current confidence threshold level.
    ///
    /// If the threshold level is lower than or equal to `confidence`, returns `Ok`.
    /// Otherwise, returns `Err`.
    pub fn assert_confidence(&self, confidence: Confidence) -> Result<(), Error> {
        if confidence as u32 >= self.confidence_threshold {
            Ok(())
        } else {
            Err(Error::new(ErrorKind::Other, "insufficient confidence"))
        }
    }

    /// Marks the `layer` as a part of the stream.
    pub fn add_layer_linkage(&mut self, token: Token, id: u64, layer: &mut Layer) {
        unsafe {
            symbol::Context_addLayerLinkage.unwrap()(self, token, id, layer);
        }
    }
}
