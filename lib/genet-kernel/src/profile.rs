use genet_abi::{
    context::Context,
    dissector::DissectorBox,
    env::{self, Allocator},
    io::{ReaderBox, WriterBox},
    ptr::{MutPtr, Ptr},
    token::Token,
};
use libloading::Library;
use std::{collections::HashMap, fmt, io, mem, ops::Deref};

#[derive(Clone)]
pub struct Profile {
    concurrency: u32,
    dissectors: Vec<DissectorBox>,
    readers: Vec<ReaderBox>,
    writers: Vec<WriterBox>,
    config: HashMap<String, String>,
}

impl fmt::Debug for Profile {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Profile")
    }
}

impl Profile {
    pub fn new() -> Profile {
        Profile {
            concurrency: 4,
            dissectors: Vec::new(),
            readers: Vec::new(),
            writers: Vec::new(),
            config: HashMap::new(),
        }
    }

    pub fn set_concurrency(&mut self, concurrency: u32) {
        if concurrency > 0 {
            self.concurrency = concurrency;
        }
    }

    pub fn concurrency(&self) -> u32 {
        self.concurrency
    }

    pub fn get_config(&self, key: &str) -> Option<String> {
        self.config.get(key).map(|s| s.to_string())
    }

    pub fn set_config(&mut self, key: &str, value: &str) {
        self.config
            .entry(String::from(key))
            .or_insert(String::from(value));
    }

    pub fn dissectors(&self) -> impl Iterator<Item = &DissectorBox> {
        self.dissectors.iter()
    }

    pub fn readers(&self) -> impl Iterator<Item = &ReaderBox> {
        self.readers.iter()
    }

    pub fn writers(&self) -> impl Iterator<Item = &WriterBox> {
        self.writers.iter()
    }

    pub fn context(&self) -> Context {
        Context::new(self.config.clone())
    }

    pub fn load_library(&mut self, path: &str) -> Result<(), io::Error> {
        let lib = Library::new(path)?;

        type FnRegisterGetToken = extern "C" fn(extern "C" fn(*const u8, u64) -> Token);
        type FnRegisterGetString = extern "C" fn(extern "C" fn(Token, *mut u64) -> *const u8);
        type FnRegisterGetAllocator = extern "C" fn(extern "C" fn() -> Ptr<Allocator>);
        type FnGetDissectors = extern "C" fn(*mut u64) -> *const DissectorBox;
        type FnGetReaders = extern "C" fn(*mut u64) -> *const ReaderBox;
        type FnGetWriters = extern "C" fn(*mut u64) -> *const WriterBox;

        {
            let func =
                unsafe { lib.get::<FnRegisterGetToken>(b"genet_abi_v1_register_get_token")? };
            func(env::abi_genet_get_token);

            let func =
                unsafe { lib.get::<FnRegisterGetString>(b"genet_abi_v1_register_get_string")? };
            func(env::abi_genet_get_string);

            let func = unsafe {
                lib.get::<FnRegisterGetAllocator>(b"genet_abi_v1_register_get_allocator")?
            };
            func(env::abi_genet_get_allocator);
        }

        if let Ok(func) = unsafe { lib.get::<FnGetDissectors>(b"genet_abi_v1_get_dissectors") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                self.dissectors
                    .push(unsafe { (*ptr.offset(i as isize)).clone() });
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetReaders>(b"genet_abi_v1_get_readers") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                self.readers
                    .push(unsafe { (*ptr.offset(i as isize)).clone() });
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetWriters>(b"genet_abi_v1_get_writers") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                self.writers
                    .push(unsafe { (*ptr.offset(i as isize)).clone() });
            }
        }

        mem::forget(lib);
        Ok(())
    }
}
