use fnv::FnvHashMap;
use lazy_static::lazy_static;
use parking_lot::Mutex;
use std::{cell::RefCell, collections::hash_map::Entry, fmt, slice, str};

/// A token value.
#[repr(C)]
#[derive(Clone, Copy, PartialEq, Eq, Hash)]
pub struct Token(u32);

impl Token {
    /// Returns a null token.
    pub fn null() -> Token {
        Token(0)
    }

    /// Returns the corresponded string.
    pub fn to_string(self) -> String {
        string(self)
    }
}

impl fmt::Debug for Token {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{} ({})", self.to_string(), self.0)
    }
}

impl Into<u32> for Token {
    fn into(self) -> u32 {
        self.0
    }
}

impl<'a> From<&'a Token> for Token {
    fn from(id: &'a Token) -> Token {
        *id
    }
}

impl From<u32> for Token {
    fn from(id: u32) -> Token {
        Token(id)
    }
}

impl From<String> for Token {
    fn from(id: String) -> Token {
        Token::from(id.as_str())
    }
}

impl From<&String> for Token {
    fn from(id: &String) -> Token {
        Token::from(id.as_str())
    }
}

impl<'a> From<&'a str> for Token {
    fn from(id: &'a str) -> Token {
        token(id)
    }
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.pad(&Token::to_string(*self))
    }
}

#[repr(C)]
pub struct TokenRegistry {
    token: unsafe extern "C" fn(*const u8, u64) -> Token,
    string: unsafe extern "C" fn(Token, *mut u64) -> *const u8,
}

impl Default for TokenRegistry {
    fn default() -> Self {
        Self {
            token: abi_genet_get_token,
            string: abi_genet_get_string,
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_abi_v1_register_token_registry(registry: TokenRegistry) {
    GENET_GET_TOKEN = registry.token;
    GENET_GET_STRING = registry.string;
}

static mut GENET_GET_TOKEN: unsafe extern "C" fn(*const u8, u64) -> Token = abi_genet_get_token;
static mut GENET_GET_STRING: unsafe extern "C" fn(Token, *mut u64) -> *const u8 =
    abi_genet_get_string;

unsafe extern "C" fn abi_genet_get_token(data: *const u8, len: u64) -> Token {
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

unsafe extern "C" fn abi_genet_get_string(token: Token, len: *mut u64) -> *const u8 {
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

fn token(id: &str) -> Token {
    if id.is_empty() {
        Token::null()
    } else {
        unsafe { GENET_GET_TOKEN(id.as_ptr(), id.len() as u64) }
    }
}

fn string(id: Token) -> String {
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
