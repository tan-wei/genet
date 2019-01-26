use crate::token::Token;
use fnv::FnvHashMap;
use lazy_static::lazy_static;
use parking_lot::Mutex;
use std::{cell::RefCell, collections::hash_map::Entry, slice, str};

#[cfg(not(feature = "genet-static"))]
#[no_mangle]
pub extern "C" fn genet_abi_version() -> u64 {
    let major: u64 = env!("CARGO_PKG_VERSION_MAJOR").parse().unwrap_or(0);
    let minor: u64 = env!("CARGO_PKG_VERSION_MINOR").parse().unwrap_or(0);
    major << 32 | minor
}

#[cfg(not(feature = "genet-static"))]
#[no_mangle]
pub extern "C" fn genet_abi_v1_register_get_token(ptr: extern "C" fn(*const u8, u64) -> Token) {
    unsafe { GENET_GET_TOKEN = ptr };
}

#[cfg(not(feature = "genet-static"))]
#[no_mangle]
pub extern "C" fn genet_abi_v1_register_get_string(
    ptr: extern "C" fn(Token, *mut u64) -> *const u8,
) {
    unsafe { GENET_GET_STRING = ptr };
}

static mut GENET_GET_TOKEN: unsafe extern "C" fn(*const u8, u64) -> Token = abi_genet_get_token;
static mut GENET_GET_STRING: unsafe extern "C" fn(Token, *mut u64) -> *const u8 =
    abi_genet_get_string;

pub unsafe extern "C" fn abi_genet_get_token(data: *const u8, len: u64) -> Token {
    let tokens = GLOBAL_TOKENS.lock();
    let mut tokens = tokens.borrow_mut();
    let strings = GLOBAL_STRINGS.lock();
    let mut strings = strings.borrow_mut();
    let id = str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize));
    if id.is_empty() {
        return Token::null();
    }
    let next = tokens.len() + 1;
    let entry = tokens.entry(String::from(id));
    if let Entry::Vacant(_) = entry {
        strings.push(String::from(id));
    }
    *entry.or_insert_with(|| Token::from(next as u32))
}

pub unsafe extern "C" fn abi_genet_get_string(token: Token, len: *mut u64) -> *const u8 {
    let strings = GLOBAL_STRINGS.lock();
    let strings = strings.borrow();
    let index: u32 = token.into();
    let index = index as usize;
    let s = if index < strings.len() {
        strings[index].as_str()
    } else {
        ""
    };
    *len = s.len() as u64;
    s.as_ptr()
}

lazy_static! {
    static ref GLOBAL_TOKENS: Mutex<RefCell<FnvHashMap<String, Token>>> =
        Mutex::new(RefCell::new(FnvHashMap::default()));
    static ref GLOBAL_STRINGS: Mutex<RefCell<Vec<String>>> =
        Mutex::new(RefCell::new(vec![String::new()]));
}

pub(crate) fn token(id: &str) -> Token {
    if id.is_empty() {
        Token::null()
    } else {
        unsafe { GENET_GET_TOKEN(id.as_ptr(), id.len() as u64) }
    }
}

pub(crate) fn string(id: Token) -> String {
    if id == Token::null() {
        String::new()
    } else {
        let mut len: u64 = 0;
        let s = unsafe { GENET_GET_STRING(id, &mut len) };
        unsafe { str::from_utf8_unchecked(slice::from_raw_parts(s, len as usize)).to_string() }
    }
}

#[cfg(test)]
mod tests {
    use crate::token::Token;

    #[test]
    fn token() {
        assert_eq!(Token::from(""), Token::null());
        let token = Token::from("eth");
        assert_eq!(token.to_string(), "eth");
        let token = Token::from("[eth]");
        assert_eq!(token.to_string(), "[eth]");
        let token = Token::from("eth");
        assert_eq!(token.to_string(), "eth");
        let token = Token::from("");
        assert_eq!(token.to_string(), "");
        let token = Token::from("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(token.to_string(), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(Token::from(1000).to_string(), "");
    }
}
