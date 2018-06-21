use context::Context;
use layer::Layer;
use ptr::MutPtr;
use result::Result;
use std::fmt;
use std::time::Duration;
use string::SafeString;

pub trait Writer: Send {
    fn new_worker(&self, ctx: &Context, args: &str) -> Box<WriterWorker>;
    fn id(&self) -> &str;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct WriterBox {
    writer: *mut Box<Writer>,
    id: extern "C" fn(*mut Box<Writer>) -> SafeString,
    new_worker: extern "C" fn(*mut Box<Writer>, *const Context, SafeString) -> WriterWorkerBox,
}

unsafe impl Send for WriterBox {}

impl WriterBox {
    pub fn new<T: 'static + Writer>(writer: T) -> WriterBox {
        let writer: Box<Writer> = Box::new(writer);
        Self {
            writer: Box::into_raw(Box::new(writer)),
            id: ffi_writer_id,
            new_worker: ffi_writer_new_worker,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.writer)
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> WriterWorkerBox {
        (self.new_worker)(self.writer, ctx, SafeString::from(args))
    }
}

extern "C" fn ffi_writer_id(writer: *mut Box<Writer>) -> SafeString {
    SafeString::from(unsafe { (*writer).id() })
}

extern "C" fn ffi_writer_new_worker(
    writer: *mut Box<Writer>,
    ctx: *const Context,
    arg: SafeString,
) -> WriterWorkerBox {
    let writer = unsafe { &*writer };
    let ctx = unsafe { &*ctx };
    WriterWorkerBox::new(writer.new_worker(ctx, arg.as_str()))
}

pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Box<ReaderWorker>;
    fn id(&self) -> &str;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ReaderBox {
    reader: *mut Box<Reader>,
    id: extern "C" fn(*mut Box<Reader>) -> SafeString,
    new_worker: extern "C" fn(*mut Box<Reader>, *const Context, SafeString) -> ReaderWorkerBox,
}

unsafe impl Send for ReaderBox {}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            id: ffi_reader_id,
            new_worker: ffi_reader_new_worker,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.reader)
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> ReaderWorkerBox {
        (self.new_worker)(self.reader, ctx, SafeString::from(args))
    }
}

extern "C" fn ffi_reader_id(reader: *mut Box<Reader>) -> SafeString {
    SafeString::from(unsafe { (*reader).id() })
}

extern "C" fn ffi_reader_new_worker(
    reader: *mut Box<Reader>,
    ctx: *const Context,
    arg: SafeString,
) -> ReaderWorkerBox {
    let reader = unsafe { &*reader };
    let ctx = unsafe { &*ctx };
    ReaderWorkerBox::new(reader.new_worker(ctx, arg.as_str()))
}

pub trait WriterWorker: Send {
    fn write(&mut self, layers: &[Layer]) -> Result<()>;
}

pub trait ReaderWorker: Send {
    fn read(&mut self, timeout: Duration) -> Result<Box<[Layer]>>;
}

pub struct WriterWorkerBox {
    worker: *mut Box<WriterWorker>,
    worker_drop: extern "C" fn(*mut Box<WriterWorker>),
}

unsafe impl Send for WriterWorkerBox {}

impl WriterWorkerBox {
    pub fn new(worker: Box<WriterWorker>) -> WriterWorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            worker_drop: ffi_writer_worker_drop,
        }
    }

    fn write(&mut self, layers: &[Layer]) -> Result<()> {
        Ok(())
    }
}

impl fmt::Debug for WriterWorkerBox {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "WriterWorkerBox")
    }
}

impl Drop for WriterWorkerBox {
    fn drop(&mut self) {
        (self.worker_drop)(self.worker);
    }
}

pub struct ReaderWorkerBox {
    worker: *mut Box<ReaderWorker>,
    worker_drop: extern "C" fn(*mut Box<ReaderWorker>),
}

unsafe impl Send for ReaderWorkerBox {}

impl ReaderWorkerBox {
    pub fn new(worker: Box<ReaderWorker>) -> ReaderWorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            worker_drop: ffi_reader_worker_drop,
        }
    }

    pub fn read(&mut self, timeout: Duration) -> Result<Vec<MutPtr<Layer>>> {
        Ok(vec![])
    }
}

impl fmt::Debug for ReaderWorkerBox {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "ReaderWorkerBox")
    }
}

impl Drop for ReaderWorkerBox {
    fn drop(&mut self) {
        (self.worker_drop)(self.worker);
    }
}

extern "C" fn ffi_writer_worker_drop(worker: *mut Box<WriterWorker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn ffi_reader_worker_drop(worker: *mut Box<ReaderWorker>) {
    unsafe { Box::from_raw(worker) };
}
