use context::Context;
use layer::Layer;
use ptr::MutPtr;
use result::Result;
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
}

unsafe impl Send for WriterBox {}

impl WriterBox {
    pub fn new<T: 'static + Writer>(writer: T) -> WriterBox {
        let writer: Box<Writer> = Box::new(writer);
        Self {
            writer: Box::into_raw(Box::new(writer)),
            id: ffi_writer_id,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.writer)
    }
}

extern "C" fn ffi_writer_id(writer: *mut Box<Writer>) -> SafeString {
    SafeString::from(unsafe { (*writer).id() })
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
}

unsafe impl Send for ReaderBox {}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            id: ffi_reader_id,
        }
    }

    pub fn id(&self) -> SafeString {
        (self.id)(self.reader)
    }
}

extern "C" fn ffi_reader_id(reader: *mut Box<Reader>) -> SafeString {
    SafeString::from(unsafe { (*reader).id() })
}

pub trait WriterWorker: Drop {
    fn write(&mut self, layers: &[Layer]) -> Result<()>;
}

pub trait ReaderWorker: Drop {
    fn read(&mut self, max: usize) -> Result<Box<[Layer]>>;
}

pub struct WriterWorkerBox {
    worker: *mut Box<WriterWorker>,
    writer_worker_drop: extern "C" fn(*mut Box<WriterWorker>),
}

impl WriterWorkerBox {
    pub fn new<T: 'static + WriterWorker>(worker: T) -> WriterWorkerBox {
        let worker: Box<WriterWorker> = Box::new(worker);
        Self {
            worker: Box::into_raw(Box::new(worker)),
            writer_worker_drop: ffi_writer_worker_drop,
        }
    }

    fn write(&mut self, layers: &[Layer]) -> Result<()> {
        Ok(())
    }
}

impl Drop for WriterWorkerBox {
    fn drop(&mut self) {
        (self.writer_worker_drop)(self.worker);
    }
}

pub struct ReaderWorkerBox {
    worker: *mut Box<ReaderWorker>,
    reader_worker_drop: extern "C" fn(*mut Box<ReaderWorker>),
}

impl ReaderWorkerBox {
    pub fn new<T: 'static + ReaderWorker>(worker: T) -> ReaderWorkerBox {
        let worker: Box<ReaderWorker> = Box::new(worker);
        Self {
            worker: Box::into_raw(Box::new(worker)),
            reader_worker_drop: ffi_reader_worker_drop,
        }
    }

    pub fn read(&mut self, max: usize) -> Result<Vec<MutPtr<Layer>>> {
        Ok(vec![])
    }
}

impl Drop for ReaderWorkerBox {
    fn drop(&mut self) {
        (self.reader_worker_drop)(self.worker);
    }
}

extern "C" fn ffi_writer_worker_drop(worker: *mut Box<WriterWorker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn ffi_reader_worker_drop(worker: *mut Box<ReaderWorker>) {
    unsafe { Box::from_raw(worker) };
}
