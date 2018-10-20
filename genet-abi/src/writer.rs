use context::Context;
use error::Error;
use file::FileType;
use fixed::MutFixed;
use layer::{Layer, LayerStack};
use result::Result;
use std::{fmt, mem, ptr};
use string::SafeString;

/// Writer metadata.
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
            id: format!("app.genet.writer.{}", env!("CARGO_PKG_NAME")),
            name: env!("CARGO_PKG_NAME").to_string(),
            description: env!("CARGO_PKG_DESCRIPTION").to_string(),
            filters: Vec::new(),
        }
    }
}

/// Writer trait.
pub trait Writer: Send {
    fn new_worker(&self, ctx: &Context, args: &str) -> Result<Box<Worker>>;
    fn id(&self) -> &str;
    fn metadata(&self) -> Metadata;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct WriterBox {
    writer: *mut Box<Writer>,
    id: extern "C" fn(*mut Box<Writer>) -> SafeString,
    new_worker:
        extern "C" fn(*mut Box<Writer>, *const Context, SafeString, *mut WorkerBox, *mut Error)
            -> u8,
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

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
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
    out: *mut WorkerBox,
    err: *mut Error,
) -> u8 {
    let writer = unsafe { &*writer };
    let ctx = unsafe { &*ctx };
    match writer.new_worker(ctx, arg.as_str()) {
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

/// Writer worker trait.
pub trait Worker: Send {
    fn write(&mut self, index: u32, stack: &LayerStack) -> Result<()>;
    fn end(&mut self) -> Result<()> {
        Ok(())
    }
}

type WriterFunc = extern "C" fn(*mut Box<Worker>, u32, *const *const Layer, u64, *mut Error) -> u8;

type WriterEndFunc = extern "C" fn(*mut Box<Worker>, *mut Error) -> u8;

pub struct WorkerBox {
    worker: *mut Box<Worker>,
    write: WriterFunc,
    end: WriterEndFunc,
    drop: extern "C" fn(*mut Box<Worker>),
}

unsafe impl Send for WorkerBox {}

impl WorkerBox {
    pub fn new(worker: Box<Worker>) -> WorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            write: abi_writer_worker_write,
            end: abi_writer_worker_end,
            drop: abi_writer_worker_drop,
        }
    }

    pub fn write(&mut self, index: u32, layers: &[MutFixed<Layer>]) -> Result<()> {
        let mut e = Error::new("");
        let stack = layers.as_ptr() as *const *const Layer;
        if (self.write)(self.worker, index, stack, layers.len() as u64, &mut e) == 0 {
            Err(Box::new(e))
        } else {
            Ok(())
        }
    }

    pub fn end(&mut self) -> Result<()> {
        let mut e = Error::new("");
        if (self.end)(self.worker, &mut e) == 0 {
            Err(Box::new(e))
        } else {
            Ok(())
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

extern "C" fn abi_writer_worker_drop(worker: *mut Box<Worker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn abi_writer_worker_write(
    worker: *mut Box<Worker>,
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

extern "C" fn abi_writer_worker_end(worker: *mut Box<Worker>, err: *mut Error) -> u8 {
    let worker = unsafe { &mut *worker };
    match worker.end() {
        Ok(()) => 1,
        Err(e) => {
            unsafe { *err = Error::new(e.description()) };
            0
        }
    }
}
