use genet_ffi::dissector::DissectorBox;
use genet_ffi::env::{self, Allocator, Env};
use genet_ffi::io::{ReaderBox, WriterBox};
use genet_ffi::ptr::{MutPtr, Ptr};
use libloading::Library;
use std::collections::HashMap;
use std::fmt;
use std::io;
use std::mem;
use std::ops::Deref;

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

    pub fn load_library(&mut self, path: &str) -> Result<(), io::Error> {
        let lib = Library::new(path)?;

        type FnRegisterGetEnv = extern "C" fn(extern "C" fn() -> MutPtr<Env>);
        type FnRegisterGetAllocator = extern "C" fn(extern "C" fn() -> Ptr<Allocator>);
        type FnGetDissectors = extern "C" fn(*mut u64) -> *const DissectorBox;
        type FnGetReaders = extern "C" fn(*mut u64) -> *const ReaderBox;
        type FnGetWriters = extern "C" fn(*mut u64) -> *const WriterBox;

        {
            let func = unsafe { lib.get::<FnRegisterGetEnv>(b"genet_ffi_v1_register_get_env")? };
            func(env::ffi_genet_get_env);

            let func = unsafe {
                lib.get::<FnRegisterGetAllocator>(b"genet_ffi_v1_register_get_allocator")?
            };
            func(env::ffi_genet_get_allocator);
        }

        if let Ok(func) = unsafe { lib.get::<FnGetDissectors>(b"genet_ffi_v1_get_dissectors") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                self.dissectors
                    .push(unsafe { (*ptr.offset(i as isize)).clone() });
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetReaders>(b"genet_ffi_v1_get_readers") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                self.readers
                    .push(unsafe { (*ptr.offset(i as isize)).clone() });
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetWriters>(b"genet_ffi_v1_get_writers") } {
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
