use ptr::MutPtr;
use std::{collections::HashMap, slice, str};

#[repr(C)]
pub struct Context {
    class: &'static ContextClass,
    abi_unsafe_data: MutPtr<ContextData>,
}

struct ContextData {
    config: HashMap<String, String>,
}

impl Context {
    pub fn new(config: HashMap<String, String>) -> Context {
        Self {
            class: &CONTEXT_CLASS,
            abi_unsafe_data: MutPtr::new(ContextData { config }),
        }
    }

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
        let val = (*ctx)
            .abi_unsafe_data
            .config
            .get(key)
            .map(|s| s.as_str())
            .unwrap_or("");
        *len = val.len() as u64;
        val.as_ptr()
    }
}

lazy_static! {
    static ref CONTEXT_CLASS: ContextClass = ContextClass::new();
}
