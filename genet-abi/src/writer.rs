use crate::{
    codable::{Codable, CodedData},
    context::Context,
    file::FileType,
    layer::Layer,
    package::{Component, IntoBuilder},
    result::Result,
    string::SafeString,
};
use failure::format_err;
use serde_derive::{Deserialize, Serialize};
use std::{fmt, mem, ptr, slice, str};

/// Writer trait.
pub trait Writer: Send {
    fn new_worker(&self, ctx: &Context, args: &str) -> Result<Box<Worker>>;
}

type WriterNewWorkerFunc = extern "C" fn(
    *mut Box<Writer>,
    *const Context,
    *const u8,
    u64,
    *mut WorkerBox,
    *mut SafeString,
) -> u8;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct WriterBox {
    writer: *mut Box<Writer>,
    new_worker: WriterNewWorkerFunc,
}

unsafe impl Send for WriterBox {}
unsafe impl Codable for WriterBox {}

impl WriterBox {
    pub fn new<T: 'static + Writer>(writer: T) -> WriterBox {
        let writer: Box<Writer> = Box::new(writer);
        Self {
            writer: Box::into_raw(Box::new(writer)),
            new_worker: abi_writer_new_worker,
        }
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
        let mut err = SafeString::new();
        if (self.new_worker)(
            self.writer,
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
            Err(format_err!("{}", err))
        }
    }
}

extern "C" fn abi_writer_new_worker(
    writer: *mut Box<Writer>,
    ctx: *const Context,
    arg: *const u8,
    arg_len: u64,
    out: *mut WorkerBox,
    err: *mut SafeString,
) -> u8 {
    let writer = unsafe { &*writer };
    let ctx = unsafe { &*ctx };
    let arg = unsafe { str::from_utf8_unchecked(slice::from_raw_parts(arg, arg_len as usize)) };
    match writer.new_worker(ctx, arg) {
        Ok(worker) => {
            unsafe { ptr::write(out, WorkerBox::new(worker)) };
            1
        }
        Err(e) => {
            unsafe { *err = SafeString::from(&format!("{}", e)) };
            0
        }
    }
}

/// Writer worker trait.
pub trait Worker: Send {
    fn write(&mut self, index: u32, layer: &Layer) -> Result<()>;
    fn end(&mut self) -> Result<()> {
        Ok(())
    }
}

type WriterFunc = extern "C" fn(*mut Box<Worker>, u32, *const Layer, *mut SafeString) -> u8;

type WriterEndFunc = extern "C" fn(*mut Box<Worker>, *mut SafeString) -> u8;

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

    pub fn write(&mut self, index: u32, layer: &Layer) -> Result<()> {
        let mut e = SafeString::new();
        let layer = layer as *const Layer;
        if (self.write)(self.worker, index, layer, &mut e) == 0 {
            Err(format_err!("{}", e))
        } else {
            Ok(())
        }
    }

    pub fn end(&mut self) -> Result<()> {
        let mut e = SafeString::new();
        if (self.end)(self.worker, &mut e) == 0 {
            Err(format_err!("{}", e))
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
    layer: *const Layer,
    err: *mut SafeString,
) -> u8 {
    let worker = unsafe { &mut *worker };
    let layer = unsafe { &*layer };
    match worker.write(index, &layer) {
        Ok(()) => 1,
        Err(e) => {
            unsafe { *err = SafeString::from(&format!("{}", e)) };
            0
        }
    }
}

extern "C" fn abi_writer_worker_end(worker: *mut Box<Worker>, err: *mut SafeString) -> u8 {
    let worker = unsafe { &mut *worker };
    match worker.end() {
        Ok(()) => 1,
        Err(e) => {
            unsafe { *err = SafeString::from(&format!("{}", e)) };
            0
        }
    }
}

impl<T: 'static + Writer> IntoBuilder<WriterBuilder> for T {
    fn into_builder(self) -> WriterBuilder {
        WriterBuilder {
            data: WriterData {
                id: String::new(),
                filters: Vec::new(),
                writer: CodedData::new(WriterBox::new(self)),
            },
        }
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct WriterData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub writer: CodedData<WriterBox>,
}

pub struct WriterBuilder {
    data: WriterData,
}

impl WriterBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn filter<T: Into<FileType>>(mut self, file: T) -> Self {
        self.data.filters.push(file.into());
        self
    }
}

impl Into<WriterData> for WriterBuilder {
    fn into(self) -> WriterData {
        self.data
    }
}

impl Into<Component> for WriterBuilder {
    fn into(self) -> Component {
        Component::Writer(self.data)
    }
}
