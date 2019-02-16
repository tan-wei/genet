use crate::{filter::CompiledFilter, token::Token};
use fnv::FnvHashMap;
use parking_lot::Mutex;
use std::{ops::Range, slice, str, sync::Arc};

pub struct Chamber {
    inner: Inner,
}

impl Chamber {
    pub fn new() -> Self {
        Self {
            inner: Inner {
                tokens: Arc::new(Mutex::new(FnvHashMap::default())),
            },
        }
    }
}

struct Inner {
    tokens: Arc<Mutex<FnvHashMap<String, Token>>>,
}

pub struct Context {
    inner: *mut Inner,
    get_token: extern "C" fn(cham: *mut Inner, s: *const u8, len: u64) -> Token,
}

impl Context {
    pub(crate) fn get_token(&self, s: &str) -> Token {
        (self.get_token)(self.inner, s.as_ptr(), s.len() as u64)
    }

    pub fn filter(&self, filter: &CompiledFilter, range: Range<usize>) -> Vec<usize> {
        vec![]
    }
}

extern "C" fn get_token(cham: *mut Inner, s: *const u8, len: u64) -> Token {
    unsafe {
        let tokens = &mut (*cham).tokens.lock();
        let key = str::from_utf8_unchecked(slice::from_raw_parts(s, len as usize));
        let index = tokens.len() as u32;
        *tokens
            .entry(key.to_string())
            .or_insert_with(|| Token::new(index, &key))
    }
}
