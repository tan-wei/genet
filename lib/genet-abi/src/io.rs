use context::Context;
use error::Error;
use layer::{Layer, LayerStack};
use ptr::MutPtr;
use result::Result;
use std::{fmt, mem, ptr};
use string::SafeString;
use vec::SafeVec;

pub trait Writer: Send {
    fn new_worker(&self, ctx: &Context, args: &str) -> Result<Box<WriterWorker>>;
    fn id(&self) -> &str;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct WriterBox {
    writer: *mut Box<Writer>,
    id: extern "C" fn(*mut Box<Writer>) -> SafeString,
    new_worker: extern "C" fn(
        *mut Box<Writer>,
        *const Context,
        SafeString,
        *mut WriterWorkerBox,
        *mut Error,
    ) -> u8,
}

unsafe impl Send for WriterBox {}

impl WriterBox {
    pub fn new<T: 'static + Writer>(writer: T) -> WriterBox {
        let writer: Box<Writer> = Box::new(writer);
        Self {
            writer: Box::into_raw(Box::new(writer)),
            id: abi_writer_id,
            new_worker: abi_writer_new_worker,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.writer)
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WriterWorkerBox> {
        let mut out: WriterWorkerBox = unsafe { mem::uninitialized() };
        let mut err = Error::new("");
        if (self.new_worker)(self.writer, ctx, SafeString::from(args), &mut out, &mut err) == 1 {
            Ok(out)
        } else {
            mem::forget(out);
            Err(Box::new(err))
        }
    }
}

extern "C" fn abi_writer_id(writer: *mut Box<Writer>) -> SafeString {
    SafeString::from(unsafe { (*writer).id() })
}

extern "C" fn abi_writer_new_worker(
    writer: *mut Box<Writer>,
    ctx: *const Context,
    arg: SafeString,
    out: *mut WriterWorkerBox,
    err: *mut Error,
) -> u8 {
    let writer = unsafe { &*writer };
    let ctx = unsafe { &*ctx };
    match writer.new_worker(ctx, arg.as_str()) {
        Ok(worker) => {
            unsafe { ptr::write(out, WriterWorkerBox::new(worker)) };
            1
        }
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}

pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Result<Box<ReaderWorker>>;
    fn id(&self) -> &str;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ReaderBox {
    reader: *mut Box<Reader>,
    id: extern "C" fn(*mut Box<Reader>) -> SafeString,
    new_worker: extern "C" fn(
        *mut Box<Reader>,
        *const Context,
        SafeString,
        *mut ReaderWorkerBox,
        *mut Error,
    ) -> u8,
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

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<ReaderWorkerBox> {
        let mut out: ReaderWorkerBox = unsafe { mem::uninitialized() };
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
    out: *mut ReaderWorkerBox,
    err: *mut Error,
) -> u8 {
    let reader = unsafe { &*reader };
    let ctx = unsafe { &*ctx };
    match reader.new_worker(ctx, arg.as_str()) {
        Ok(worker) => {
            unsafe { ptr::write(out, ReaderWorkerBox::new(worker)) };
            1
        }
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}

pub trait WriterWorker: Send {
    fn write(&mut self, index: u32, stack: &LayerStack) -> Result<()>;
}

pub trait ReaderWorker: Send {
    fn read(&mut self) -> Result<Vec<Layer>>;
}

pub struct WriterWorkerBox {
    worker: *mut Box<WriterWorker>,
    write: extern "C" fn(*mut Box<WriterWorker>, u32, *const *const Layer, u64, *mut Error) -> u8,
    drop: extern "C" fn(*mut Box<WriterWorker>),
}

unsafe impl Send for WriterWorkerBox {}

impl WriterWorkerBox {
    pub fn new(worker: Box<WriterWorker>) -> WriterWorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            write: abi_writer_worker_write,
            drop: abi_writer_worker_drop,
        }
    }

    pub fn write(&mut self, index: u32, layers: &[MutPtr<Layer>]) -> Result<()> {
        let mut e = Error::new("");
        let stack = unsafe { mem::transmute(layers.as_ptr()) };
        if (self.write)(self.worker, index, stack, layers.len() as u64, &mut e) == 0 {
            Err(Box::new(e))
        } else {
            Ok(())
        }
    }
}

impl fmt::Debug for WriterWorkerBox {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "WriterWorkerBox")
    }
}

impl Drop for WriterWorkerBox {
    fn drop(&mut self) {
        (self.drop)(self.worker);
    }
}

pub struct ReaderWorkerBox {
    worker: *mut Box<ReaderWorker>,
    read: extern "C" fn(*mut Box<ReaderWorker>, *mut SafeVec<MutPtr<Layer>>, *mut Error) -> u8,
    drop: extern "C" fn(*mut Box<ReaderWorker>),
}

unsafe impl Send for ReaderWorkerBox {}

impl ReaderWorkerBox {
    pub fn new(worker: Box<ReaderWorker>) -> ReaderWorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            read: abi_reader_worker_read,
            drop: abi_reader_worker_drop,
        }
    }

    pub fn read(&mut self) -> Result<Vec<MutPtr<Layer>>> {
        let mut v = SafeVec::new();
        let mut e = Error::new("");
        if (self.read)(self.worker, &mut v, &mut e) == 0 {
            Err(Box::new(e))
        } else {
            Ok(v.into_iter().collect())
        }
    }
}

impl fmt::Debug for ReaderWorkerBox {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "ReaderWorkerBox")
    }
}

impl Drop for ReaderWorkerBox {
    fn drop(&mut self) {
        (self.drop)(self.worker);
    }
}

extern "C" fn abi_writer_worker_drop(worker: *mut Box<WriterWorker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn abi_reader_worker_drop(worker: *mut Box<ReaderWorker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn abi_writer_worker_write(
    worker: *mut Box<WriterWorker>,
    index: u32,
    layers: *const *const Layer,
    len: u64,
    err: *mut Error,
) -> u8 {
    let worker = unsafe { &mut *worker };
    let stack = unsafe { LayerStack::new(layers, len as usize) };
    match worker.write(index, &stack) {
        Ok(()) => 1,
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}

extern "C" fn abi_reader_worker_read(
    worker: *mut Box<ReaderWorker>,
    out: *mut SafeVec<MutPtr<Layer>>,
    err: *mut Error,
) -> u8 {
    let worker = unsafe { &mut *worker };
    match worker.read() {
        Ok(layers) => {
            let mut safe = SafeVec::with_capacity(layers.len() as u64);
            for l in layers.into_iter() {
                safe.push(MutPtr::new(l));
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
