extern crate libc;

use std::mem;
use std::slice;
use std::io::{Error, ErrorKind, Result};
use std::path::Path;
use super::layer::Layer;

pub struct RawFrame {
    link: u32,
    data: *const libc::c_char,
    len: u64,
    actlen: u64,
    ts_sec: u64,
    ts_nsec: u64,
    root: *const Layer
}

impl RawFrame {
    pub fn link(&self) -> u32 {
        self.link
    }

    pub fn set_link(&mut self, val: u32) {
        self.link = val;
    }

    pub fn data(&self) -> &[u8] {
        unsafe { &slice::from_raw_parts(self.data as *const u8, self.len as usize) }
    }

    pub fn set_data_and_forget(&mut self, data: Box<[u8]>) {
        self.data = data.as_ptr() as *const i8;
        self.len = data.len() as u64;
        mem::forget(data);
    }

    pub fn actlen(&self) -> u64 {
        self.actlen
    }

    pub fn set_actlen(&mut self, val: u64) {
        self.actlen = val;
    }

    pub fn ts(&self) -> (u64, u64) {
        (self.ts_sec, self.ts_nsec)
    }

    pub fn set_ts(&mut self, val: (u64, u64)) {
        self.ts_sec = val.0;
        self.ts_nsec = val.1;
    }

    pub fn root(&self) -> Option<&Layer> {
        if self.root.is_null() {
            None
        } else {
            unsafe { Some(&*self.root) }
        }
    }
}

pub trait Exp {
    fn start() -> Result<(f32)> {
        Err(Error::new(ErrorKind::InvalidInput, "unsupported"))
    }
}

pub trait Exporter {
    fn open(&mut self, _path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run(&mut self, &[RawFrame]) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}

pub trait Importer<'a> {
    fn open(&mut self, _path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run(&mut self) -> Result<(&'a [RawFrame], f32)> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}
