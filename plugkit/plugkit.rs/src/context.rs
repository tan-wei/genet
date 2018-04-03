//! Theread context.
//!
//! Type Context holds data for thread-local features such as allocator and logger.

use super::layer::{Confidence, Layer};
use super::symbol;
use super::token::Token;
use std::io::{Error, ErrorKind};
use std::str;
use std::ffi::CStr;
extern crate serde_json;
use self::serde_json::Value;
use std::sync::{Arc, Mutex};

/// A Context object.
#[repr(C)]
pub struct Context {
    close_stream: bool,
    confidence_threshold: u32,
}

impl Context {
    /// Returns a config value in the current profile.
    pub fn get_config(&self, name: &str) -> Value {
        unsafe {
            let slice = CStr::from_ptr(symbol::Context_getConfig.unwrap()(
                self,
                name.as_ptr() as *const i8,
                name.len(),
            ));
            let v: Result<Value, self::serde_json::Error> =
                serde_json::from_str(str::from_utf8_unchecked(slice.to_bytes()));
            v.unwrap_or_else(|_| Value::Null)
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


pub struct SharedContextWrapper {
    _shared: Arc<Mutex<SharedContext>>,
}

impl SharedContextWrapper {
    fn new() -> SharedContextWrapper {
        let shared = Arc::new(Mutex::new(SharedContext::new()));
        SharedContextWrapper{
            _shared: shared
        }
    }
}

pub struct SharedContext {

}

impl SharedContext {
    fn new() -> SharedContext {
        SharedContext{
           
        }
    }
}

#[no_mangle]
pub extern "C" fn plugkit_in_create_shared_ctx() -> *mut SharedContextWrapper {
    Box::into_raw(Box::new(SharedContextWrapper::new()))
}

#[no_mangle]
pub extern "C" fn plugkit_in_destroy_shared_ctx(ctx: *mut SharedContextWrapper) {
    unsafe {
        Box::from_raw(ctx);
    }
}