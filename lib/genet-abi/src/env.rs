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
pub extern "C" fn genet_abi_v1_register_get_token(ptr: extern "C" fn(*const u8, u64) -> Token) {
    unsafe { GENET_GET_TOKEN = ptr };
}

#[no_mangle]
pub extern "C" fn genet_abi_v1_register_get_string(
    ptr: extern "C" fn(Token, *mut u64) -> *const u8,
) {
    unsafe { GENET_GET_STRING = ptr };
}

#[no_mangle]
pub extern "C" fn genet_abi_v1_register_get_allocator(ptr: extern "C" fn() -> Ptr<Allocator>) {
    unsafe { GENET_GET_ALLOCATOR = ptr };
}

static mut GENET_GET_TOKEN: extern "C" fn(*const u8, u64) -> Token = abi_genet_get_token;
static mut GENET_GET_STRING: extern "C" fn(Token, *mut u64) -> *const u8 = abi_genet_get_string;
static mut GENET_GET_ALLOCATOR: extern "C" fn() -> Ptr<Allocator> = abi_genet_get_allocator;

pub extern "C" fn abi_genet_get_allocator() -> Ptr<Allocator> {
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

pub extern "C" fn abi_genet_get_token(data: *const u8, len: u64) -> Token {
    let tokens = GLOBAL_TOKENS.lock().unwrap();
    let mut tokens = tokens.borrow_mut();
    let strings = GLOBAL_STRINGS.lock().unwrap();
    let mut strings = strings.borrow_mut();
    let id = unsafe { str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize)) };
    if id.is_empty() {
        return Token::null();
    }
    let next = tokens.len() + 1;
    let entry = tokens.entry(String::from(id));
    if let Entry::Vacant(_) = entry {
        strings.push(String::from(id));
    }
    *entry.or_insert(Token::new(next as u64))
}

pub extern "C" fn abi_genet_get_string(token: Token, len: *mut u64) -> *const u8 {
    let strings = GLOBAL_STRINGS.lock().unwrap();
    let strings = strings.borrow();
    let index = token.as_u64() as usize;
    let s = if index < strings.len() {
        strings[index].as_str()
    } else {
        ""
    };
    unsafe { *len = s.len() as u64 }
    s.as_ptr()
}

lazy_static! {
    static ref GLOBAL_ALLOCATOR: Ptr<Allocator> = unsafe { GENET_GET_ALLOCATOR() };
    static ref GLOBAL_TOKENS: Mutex<RefCell<HashMap<String, Token>>> =
        Mutex::new(RefCell::new(HashMap::new()));
    static ref GLOBAL_STRINGS: Mutex<RefCell<Vec<String>>> =
        Mutex::new(RefCell::new(vec![String::new()]));
}

#[repr(C)]
pub struct Allocator {
    alloc: extern "C" fn(u64) -> *mut u8,
    realloc: extern "C" fn(*mut u8, u64) -> *mut u8,
    dealloc: extern "C" fn(*mut u8),
}

pub fn token(id: &str) -> Token {
    unsafe { GENET_GET_TOKEN(id.as_ptr(), id.len() as u64) }
}

pub fn string(id: Token) -> String {
    let mut len: u64 = 0;
    let s = unsafe { GENET_GET_STRING(id, &mut len) };
    unsafe { str::from_utf8_unchecked(slice::from_raw_parts(s, len as usize)).to_string() }
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
    use token::Token;

    #[test]
    fn token() {
        assert_eq!(env::token(""), Token::null());
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
        assert_eq!(env::string(Token::new(1000)), "");
    }
}
