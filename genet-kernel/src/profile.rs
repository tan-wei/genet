use bincode;
use failure::{err_msg, Error};
use fnv::FnvHashMap;
use genet_abi::{
    context::Context,
    decoder::DecoderBox,
    env::{self, Allocator},
    fixed::Fixed,
    package::{Component, DecoderData, Package, ReaderData, WriterData},
    reader::ReaderBox,
    token::Token,
    writer::WriterBox,
};
use libloading::Library;
use num_cpus;
use serde_derive::Serialize;
use std::{mem, slice};

#[derive(Serialize, Clone, Default)]
pub struct Profile {
    concurrency: u32,
    decoders: Vec<DecoderData>,
    readers: Vec<ReaderData>,
    writers: Vec<WriterData>,
    config: FnvHashMap<String, String>,
}

impl Profile {
    pub fn new() -> Profile {
        Profile {
            concurrency: 4,
            decoders: Vec::new(),
            readers: Vec::new(),
            writers: Vec::new(),
            config: FnvHashMap::default(),
        }
    }

    pub fn set_concurrency(&mut self, concurrency: u32) {
        self.concurrency = if concurrency > 0 {
            concurrency
        } else {
            num_cpus::get() as u32
        };
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
            .or_insert_with(|| String::from(value));
    }

    pub fn decoders(&self) -> impl Iterator<Item = &DecoderData> {
        self.decoders.iter()
    }

    pub fn readers(&self) -> impl Iterator<Item = &ReaderData> {
        self.readers.iter()
    }

    pub fn writers(&self) -> impl Iterator<Item = &WriterData> {
        self.writers.iter()
    }

    pub fn context(&self) -> Context {
        let mut ctx = Context::default();
        ctx.set_decoders(self.decoders.clone());
        ctx
    }

    pub fn load_library(&mut self, path: &str) -> Result<(), Error> {
        let lib = Library::new(path)?;

        type FnVersion = extern "C" fn() -> u64;
        type FnRegisterGetToken = extern "C" fn(unsafe extern "C" fn(*const u8, u64) -> Token);
        type FnRegisterGetString =
            extern "C" fn(unsafe extern "C" fn(Token, *mut u64) -> *const u8);
        type FnRegisterGetAllocator = extern "C" fn(extern "C" fn() -> Fixed<Allocator>);
        type FnGetDecoders = extern "C" fn(*mut u64) -> *const DecoderBox;
        type FnGetReaders = extern "C" fn(*mut u64) -> *const ReaderBox;
        type FnGetWriters = extern "C" fn(*mut u64) -> *const WriterBox;

        type FnLoadPackageCallback = extern "C" fn(*const u8, u64, *mut Vec<u8>);
        type FnLoadPackage = extern "C" fn(*mut Vec<u8>, FnLoadPackageCallback);

        {
            let func = unsafe { lib.get::<FnVersion>(b"genet_abi_version")? };

            // In the initial development, minor version changes may break ABI.
            fn canonical(ver: u64) -> u64 {
                if ver >> 32 == 0 {
                    ver
                } else {
                    ver & (0xffff_ffff << 32)
                }
            }

            if canonical(env::genet_abi_version()) != canonical(func()) {
                return Err(err_msg("abi version mismatch"));
            }

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

        if let Ok(func) = unsafe { lib.get::<FnGetDecoders>(b"genet_abi_v1_get_decoders") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                let b = unsafe { (*ptr.offset(i as isize)) };
                let id = b.metadata().id.clone();
                let trigger_after = b.metadata().trigger_after.clone();
                let mut b = DecoderData::builder(b).id(id);
                for t in trigger_after.into_iter() {
                    b = b.trigger_after(t);
                }
                self.decoders.push(b.into());
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetReaders>(b"genet_abi_v1_get_readers") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                let b = unsafe { (*ptr.offset(i as isize)) };
                let id = b.metadata().id.clone();
                let filters = b.metadata().filters.clone();
                let mut b = ReaderData::builder(b).id(id);
                for t in filters.into_iter() {
                    b = b.filter(t);
                }
                self.readers.push(b.into());
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnGetWriters>(b"genet_abi_v1_get_writers") } {
            let mut len = 0;
            let ptr = func(&mut len);
            for i in 0..len {
                let b = unsafe { (*ptr.offset(i as isize)) };
                let id = b.metadata().id.clone();
                let filters = b.metadata().filters.clone();
                let mut b = WriterData::builder(b).id(id);
                for t in filters.into_iter() {
                    b = b.filter(t);
                }
                self.writers.push(b.into());
            }
        }

        if let Ok(func) = unsafe { lib.get::<FnLoadPackage>(b"genet_abi_v1_load_package") } {
            extern "C" fn cb(data: *const u8, len: u64, dst: *mut Vec<u8>) {
                unsafe { *dst = slice::from_raw_parts(data, len as usize).to_vec() };
            }
            let mut buf = Vec::new();
            func(&mut buf, cb);
            if let Ok(pkg) = bincode::deserialize::<Package>(&buf) {
                for cmp in pkg.components {
                    match cmp {
                        Component::Decoder(data) => self.decoders.push(data),
                        Component::Reader(data) => self.readers.push(data),
                        Component::Writer(data) => self.writers.push(data),
                    }
                }
            }
        }

        mem::forget(lib);
        Ok(())
    }
}
