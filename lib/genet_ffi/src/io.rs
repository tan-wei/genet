use context::Context;
use layer::Layer;
use ptr::MutPtr;
use result::Result;

pub trait Writer {
    fn new_worker(&self, ctx: &Context, args: &str) -> Box<WriterWorker>;
}

pub struct WriterBox {
    writer: *mut Box<Writer>,
}

pub trait Reader {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Box<ReaderWorker>;
}

pub struct ReaderBox {
    reader: *mut Box<Reader>,
}

pub trait WriterWorker: Drop {
    fn write(&mut self, layers: &[Layer]) -> Result<()>;
}

pub trait ReaderWorker: Drop {
    fn read(&mut self, max: usize) -> Result<Box<[Layer]>>;
}

pub struct WriterWorkerBox {
    worker: *mut Box<WriterWorker>,
    drop_writer_worker: extern "C" fn(*mut Box<WriterWorker>),
}

impl WriterWorkerBox {
    pub fn new<T: 'static + WriterWorker>(worker: T) -> WriterWorkerBox {
        let worker: Box<WriterWorker> = Box::new(worker);
        Self {
            worker: Box::into_raw(Box::new(worker)),
            drop_writer_worker: ffi_drop_writer_worker,
        }
    }

    fn write(&mut self, layers: &[Layer]) -> Result<()> {
        Ok(())
    }
}

impl Drop for WriterWorkerBox {
    fn drop(&mut self) {
        (self.drop_writer_worker)(self.worker);
    }
}

pub struct ReaderWorkerBox {
    worker: *mut Box<ReaderWorker>,
    drop_reader_worker: extern "C" fn(*mut Box<ReaderWorker>),
}

impl ReaderWorkerBox {
    pub fn new<T: 'static + ReaderWorker>(worker: T) -> ReaderWorkerBox {
        let worker: Box<ReaderWorker> = Box::new(worker);
        Self {
            worker: Box::into_raw(Box::new(worker)),
            drop_reader_worker: ffi_drop_reader_worker,
        }
    }

    pub fn read(&mut self, max: usize) -> Result<Vec<MutPtr<Layer>>> {
        Ok(vec![])
    }
}

impl Drop for ReaderWorkerBox {
    fn drop(&mut self) {
        (self.drop_reader_worker)(self.worker);
    }
}

extern "C" fn ffi_drop_writer_worker(worker: *mut Box<WriterWorker>) {
    unsafe { Box::from_raw(worker) };
}

extern "C" fn ffi_drop_reader_worker(worker: *mut Box<ReaderWorker>) {
    unsafe { Box::from_raw(worker) };
}
