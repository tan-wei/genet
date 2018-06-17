use libc;
use ptr::{MutPtr, Ptr};
use std::cell::RefCell;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::slice;
use std::str;
use std::sync::Mutex;
use token::Token;

#[no_mangle]
pub extern "C" fn dyr_ffi_v1_register_get_env(ptr: extern "C" fn() -> MutPtr<Env>) {
    unsafe { DYR_GET_ENV = ptr };
}

#[no_mangle]
pub extern "C" fn dyr_ffi_v1_register_get_allocator(ptr: extern "C" fn() -> Ptr<Allocator>) {
    unsafe { DYR_GET_ALLOCATOR = ptr };
}

static mut DYR_GET_ENV: extern "C" fn() -> MutPtr<Env> = ffi_dyr_get_env;
static mut DYR_GET_ALLOCATOR: extern "C" fn() -> Ptr<Allocator> = ffi_dyr_get_allocator;

pub extern "C" fn ffi_dyr_get_env() -> MutPtr<Env> {
    MutPtr::new(Env {
        rev: Revision::String,
        data: MutPtr::new(EnvData {
            tokens: HashMap::new(),
            strings: vec![String::new()],
        }),
        token: ffi_token,
        string: ffi_string,
    })
}

pub extern "C" fn ffi_dyr_get_allocator() -> Ptr<Allocator> {
    extern "C" fn alloc(size: u64) -> *mut u8 {
        unsafe { libc::malloc(size as usize) as *mut u8 }
    }
    extern "C" fn realloc(ptr: *mut u8, size: u64) -> *mut u8 {
        unsafe { libc::realloc(ptr as *mut libc::c_void, size as usize) as *mut u8 }
    }
    extern "C" fn dealloc(ptr: *mut u8) {
        unsafe { libc::free(ptr as *mut libc::c_void) };
    }
    Ptr::new(Allocator {
        alloc,
        realloc,
        dealloc,
    })
}

lazy_static! {
    static ref GLOBAL_ENV: Mutex<RefCell<MutPtr<Env>>> =
        unsafe { Mutex::new(RefCell::new(DYR_GET_ENV())) };
    static ref GLOBAL_ALLOCATOR: Ptr<Allocator> = unsafe { DYR_GET_ALLOCATOR() };
}

#[repr(C)]
pub struct Allocator {
    alloc: extern "C" fn(u64) -> *mut u8,
    realloc: extern "C" fn(*mut u8, u64) -> *mut u8,
    dealloc: extern "C" fn(*mut u8),
}

#[repr(u64)]
#[derive(PartialEq, PartialOrd)]
#[allow(dead_code)]
enum Revision {
    String = 1,
}

#[repr(C)]
pub struct Env {
    rev: Revision,
    data: MutPtr<EnvData>,
    token: extern "C" fn(*mut EnvData, *const u8, u64) -> Token,
    string: extern "C" fn(*const EnvData, Token, *mut u64) -> *const u8,
}

struct EnvData {
    tokens: HashMap<String, Token>,
    strings: Vec<String>,
}

impl Env {
    pub fn token(&mut self, id: &str) -> Token {
        (self.token)(self.data.as_mut_ptr(), id.as_ptr(), id.len() as u64)
    }

    pub fn string(&self, id: Token) -> String {
        let mut len: u64 = 0;
        let s = (self.string)(self.data.as_ptr(), id, &mut len);
        unsafe { str::from_utf8_unchecked(slice::from_raw_parts(s, len as usize)).to_string() }
    }
}

extern "C" fn ffi_token(env: *mut EnvData, data: *const u8, len: u64) -> Token {
    let tokens = unsafe { &mut (*env).tokens };
    let strings = unsafe { &mut (*env).strings };
    let id = unsafe { str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize)) };
    if id.is_empty() {
        return 0;
    }
    let next = tokens.len() + 1;
    let entry = tokens.entry(String::from(id));
    if let Entry::Vacant(_) = entry {
        strings.push(String::from(id));
    }
    *entry.or_insert(next as u64)
}

extern "C" fn ffi_string(env: *const EnvData, token: Token, len: *mut u64) -> *const u8 {
    let strings = unsafe { &(*env).strings };
    let index = token as usize;
    let s = if index < strings.len() {
        strings[index].as_str()
    } else {
        ""
    };
    unsafe { *len = s.len() as u64 }
    s.as_ptr()
}

pub fn token(id: &str) -> Token {
    let env = GLOBAL_ENV.lock().unwrap();
    let mut env = env.borrow_mut();
    env.token(id)
}

pub fn string(id: Token) -> String {
    let env = GLOBAL_ENV.lock().unwrap();
    let env = env.borrow();
    env.string(id)
}

pub fn alloc(len: usize) -> *mut u8 {
    (GLOBAL_ALLOCATOR.alloc)(len as u64)
}

pub fn realloc(ptr: *mut u8, len: usize) -> *mut u8 {
    (GLOBAL_ALLOCATOR.realloc)(ptr, len as u64)
}

pub fn dealloc(ptr: *mut u8) {
    (GLOBAL_ALLOCATOR.dealloc)(ptr)
}

#[cfg(test)]
mod tests {
    use env;

    #[test]
    fn token() {
        assert_eq!(env::token(""), 0);
        let token = env::token("eth");
        assert_eq!(env::string(token), "eth");
        let token = env::token("[eth]");
        assert_eq!(env::string(token), "[eth]");
        let token = env::token("eth");
        assert_eq!(env::string(token), "eth");
        let token = env::token("");
        assert_eq!(env::string(token), "");
        let token = env::token("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(token), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(1000), "");
    }
}
