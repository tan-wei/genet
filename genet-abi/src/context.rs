use fixed::Fixed;
use fnv::FnvHashMap;
use std::{slice, str};

/// A context object.
#[repr(C)]
pub struct Context {
    class: Fixed<ContextClass>,
    config: FnvHashMap<String, String>,
}

impl Context {
    /// Creates a new Context.
    pub fn new(config: FnvHashMap<String, String>) -> Context {
        Self {
            class: *CONTEXT_CLASS,
            config,
        }
    }

    /// Returns a config value in the current profile.
    pub fn get_config(&self, key: &str) -> &str {
        let mut len = key.len() as u64;
        let data = (self.class.get_config)(self, key.as_ptr(), &mut len);
        unsafe { str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize)) }
    }
}

#[repr(C)]
pub struct ContextClass {
    get_config: extern "C" fn(*const Context, *const u8, *mut u64) -> *const u8,
}

impl ContextClass {
    fn new() -> ContextClass {
        Self {
            get_config: abi_get_config,
        }
    }
}

extern "C" fn abi_get_config(ctx: *const Context, data: *const u8, len: *mut u64) -> *const u8 {
    unsafe {
        let key = str::from_utf8_unchecked(slice::from_raw_parts(data, *len as usize));
        let val = (*ctx).config.get(key).map(|s| s.as_str()).unwrap_or("");
        *len = val.len() as u64;
        val.as_ptr()
    }
}

lazy_static! {
    static ref CONTEXT_CLASS: Fixed<ContextClass> = Fixed::new(ContextClass::new());
}
