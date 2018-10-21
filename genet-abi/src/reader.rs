use bincode;
use context::Context;
use error::Error;
use file::FileType;
use fixed::MutFixed;
use layer::Layer;
use result::Result;
use serde::ser::{Serialize, Serializer};
use std::{fmt, mem, ptr, slice, str};
use vec::SafeVec;

/// Reader metadata.
#[derive(Serialize, Deserialize, Debug)]
pub struct Metadata {
    pub id: String,
    pub name: String,
    pub description: String,
    pub filters: Vec<FileType>,
}

impl Default for Metadata {
    fn default() -> Self {
        Metadata {
            id: String::new(),
            name: String::new(),
            description: String::new(),
            filters: Vec::new(),
        }
    }
}

/// Reader trait.
pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Result<Box<Worker>>;
    fn metadata(&self) -> Metadata;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ReaderBox {
    reader: *mut Box<Reader>,
    new_worker:
        extern "C" fn(*mut Box<Reader>, *const Context, *const u8, u64, *mut WorkerBox, *mut Error)
            -> u8,
    metadata: extern "C" fn(*const ReaderBox) -> SafeVec<u8>,
}

unsafe impl Send for ReaderBox {}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            new_worker: abi_reader_new_worker,
            metadata: abi_metadata,
        }
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
        let mut err = Error::new("");
        if (self.new_worker)(
            self.reader,
            ctx,
            args.as_ptr(),
            args.len() as u64,
            &mut out,
            &mut err,
        ) == 1
        {
            Ok(out)
        } else {
            mem::forget(out);
            Err(Box::new(err))
        }
    }

    pub fn metadata(&self) -> Metadata {
        bincode::deserialize(&(self.metadata)(self)).unwrap()
    }
}

impl Serialize for ReaderBox {
    fn serialize<S>(&self, serializer: S) -> ::std::result::Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        self.metadata().serialize(serializer)
    }
}

extern "C" fn abi_reader_new_worker(
    reader: *mut Box<Reader>,
    ctx: *const Context,
    arg: *const u8,
    arg_len: u64,
    out: *mut WorkerBox,
    err: *mut Error,
) -> u8 {
    let reader = unsafe { &*reader };
    let ctx = unsafe { &*ctx };
    let arg = unsafe { str::from_utf8_unchecked(slice::from_raw_parts(arg, arg_len as usize)) };
    match reader.new_worker(ctx, arg) {
        Ok(worker) => {
            unsafe { ptr::write(out, WorkerBox::new(worker)) };
            1
        }
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}

extern "C" fn abi_metadata(reader: *const ReaderBox) -> SafeVec<u8> {
    let reader = unsafe { &*((*reader).reader) };
    bincode::serialize(&reader.metadata()).unwrap().into()
}

/// Reader worker trait.
pub trait Worker: Send {
    fn read(&mut self) -> Result<Vec<Layer>>;
}

type ReaderFunc = extern "C" fn(*mut Box<Worker>, *mut SafeVec<MutFixed<Layer>>, *mut Error) -> u8;

pub struct WorkerBox {
    worker: *mut Box<Worker>,
    read: ReaderFunc,
    drop: extern "C" fn(*mut Box<Worker>),
}

unsafe impl Send for WorkerBox {}

impl WorkerBox {
    pub fn new(worker: Box<Worker>) -> WorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            read: abi_reader_worker_read,
            drop: abi_reader_worker_drop,
        }
    }

    pub fn read(&mut self) -> Result<Vec<MutFixed<Layer>>> {
        let mut v = SafeVec::new();
        let mut e = Error::new("");
        if (self.read)(self.worker, &mut v, &mut e) == 0 {
            Err(Box::new(e))
        } else {
            Ok(v.into_iter().collect())
        }
    }
}

impl fmt::Debug for WorkerBox {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "WorkerBox")
    }
}

impl Drop for WorkerBox {
    fn drop(&mut self) {
        (self.drop)(self.worker);
    }
}

extern "C" fn abi_reader_worker_drop(worker: *mut Box<Worker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn abi_reader_worker_read(
    worker: *mut Box<Worker>,
    out: *mut SafeVec<MutFixed<Layer>>,
    err: *mut Error,
) -> u8 {
    let worker = unsafe { &mut *worker };
    match worker.read() {
        Ok(layers) => {
            let mut safe = SafeVec::with_capacity(layers.len() as u64);
            for layer in layers {
                safe.push(MutFixed::new(layer));
            }
            unsafe { *out = safe };
            1
        }
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}
