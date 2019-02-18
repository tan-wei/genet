use fnv::FnvHashMap;
use lazy_static::lazy_static;
use parking_lot::Mutex;
use std::{
    cell::RefCell,
    fmt, mem,
    ptr::{self, NonNull},
    slice, str,
};

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Token(Option<NonNull<Header>>);

unsafe impl Send for Token {}
unsafe impl Sync for Token {}

impl fmt::Debug for Token {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{} ({})", self.as_str(), self.index())
    }
}

impl PartialEq for Token {
    fn eq(&self, other: &Token) -> bool {
        self.0 == other.0 || self.index() == other.index()
    }
}

impl Eq for Token {}

impl Token {
    pub fn null() -> Self {
        Self(None)
    }

    pub(crate) fn new(index: u32, s: &str) -> Self {
        let ptr;

        if s.is_empty() {
            let header = Header { index, strlen: 0 };
            ptr = Box::into_raw(Box::new(header));
        } else {
            let header = Header {
                index,
                strlen: s.len() as u32,
            };
            let header_size = mem::size_of::<Header>();
            let size = 1 + (s.len() + header_size - 1) / header_size;
            let mut v = vec![header; size];
            let bytes = s.as_bytes();
            unsafe {
                ptr::copy_nonoverlapping(
                    bytes.as_ptr(),
                    v.as_mut_ptr().offset(1) as *mut u8,
                    bytes.len(),
                );
            }
            ptr = v.as_mut_ptr();
            mem::forget(v);
        }
        unsafe { Self(Some(NonNull::new_unchecked(ptr))) }
    }

    pub fn as_str(&self) -> &str {
        if let Some(header) = self.header() {
            if header.strlen == 0 {
                ""
            } else {
                unsafe {
                    let data = self.0.unwrap().as_ptr().offset(1) as *const u8;
                    str::from_utf8_unchecked(slice::from_raw_parts(data, header.strlen as usize))
                }
            }
        } else {
            ""
        }
    }

    pub fn index(&self) -> u32 {
        if let Some(header) = self.header() {
            header.index
        } else {
            0
        }
    }

    fn header(&self) -> Option<&Header> {
        unsafe {
            if let Some(header) = &self.0 {
                Some(header.as_ref())
            } else {
                None
            }
        }
    }
}

impl<'a> From<&'a str> for Token {
    fn from(id: &'a str) -> Token {
        token(id)
    }
}

impl From<&String> for Token {
    fn from(id: &String) -> Token {
        Token::from(id.as_str())
    }
}

impl From<String> for Token {
    fn from(id: String) -> Token {
        Token::from(id.as_str())
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
struct Header {
    pub index: u32,
    pub strlen: u32,
}

#[repr(C)]
pub struct TokenRegistry {
    token: unsafe extern "C" fn(*const u8, u64) -> Token,
}

impl Default for TokenRegistry {
    fn default() -> Self {
        Self {
            token: abi_genet_get_token,
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_abi_v1_register_token_registry(registry: TokenRegistry) {
    GENET_GET_TOKEN = registry.token;
}

static mut GENET_GET_TOKEN: unsafe extern "C" fn(*const u8, u64) -> Token = abi_genet_get_token;

unsafe extern "C" fn abi_genet_get_token(data: *const u8, len: u64) -> Token {
    let tokens = GLOBAL_TOKENS.lock();
    let mut tokens = tokens.borrow_mut();
    let id = str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize));
    if id.is_empty() {
        return Token::null();
    }
    let next = tokens.len() + 1;
    let entry = tokens.entry(String::from(id));
    *entry.or_insert_with(|| Token::new(next as u32, id))
}

lazy_static! {
    static ref GLOBAL_TOKENS: Mutex<RefCell<FnvHashMap<String, Token>>> =
        Mutex::new(RefCell::new(FnvHashMap::default()));
}

fn token(id: &str) -> Token {
    if id.is_empty() {
        Token::null()
    } else {
        unsafe { GENET_GET_TOKEN(id.as_ptr(), id.len() as u64) }
    }
}
