use context::Context;
use error::Error;
use fixed::MutFixed;
use layer::Layer;
use result::Result;
use std::{fmt, mem, ptr};
use string::SafeString;
use vec::SafeVec;

/// Reader trait.
pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Result<Box<Worker>>;
    fn id(&self) -> &str;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ReaderBox {
    reader: *mut Box<Reader>,
    id: extern "C" fn(*mut Box<Reader>) -> SafeString,
    new_worker:
        extern "C" fn(*mut Box<Reader>, *const Context, SafeString, *mut WorkerBox, *mut Error)
            -> u8,
}

unsafe impl Send for ReaderBox {}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            id: abi_reader_id,
            new_worker: abi_reader_new_worker,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.reader)
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
        let mut err = Error::new("");
        if (self.new_worker)(self.reader, ctx, SafeString::from(args), &mut out, &mut err) == 1 {
            Ok(out)
        } else {
            mem::forget(out);
            Err(Box::new(err))
        }
    }
}

extern "C" fn abi_reader_id(reader: *mut Box<Reader>) -> SafeString {
    SafeString::from(unsafe { (*reader).id() })
}

extern "C" fn abi_reader_new_worker(
    reader: *mut Box<Reader>,
    ctx: *const Context,
    arg: SafeString,
    out: *mut WorkerBox,
    err: *mut Error,
) -> u8 {
    let reader = unsafe { &*reader };
    let ctx = unsafe { &*ctx };
    match reader.new_worker(ctx, arg.as_str()) {
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
