use crate::{
    codable::{Codable, CodedData},
    context::Context,
    file::FileType,
    package::{Component, IntoBuilder},
    result::Result,
    slice::ByteSlice,
    string::SafeString,
    token::Token,
    vec::SafeVec,
};
use failure::format_err;
use serde_derive::{Deserialize, Serialize};
use std::{fmt, mem, ptr, slice, str};

/// Reader trait.
pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, arg: &str) -> Result<Box<Worker>>;
}

type ReaderNewWorkerFunc = extern "C" fn(
    *mut Box<Reader>,
    *const Context,
    *const u8,
    u64,
    *mut WorkerBox,
    *mut SafeString,
) -> u8;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ReaderBox {
    reader: *mut Box<Reader>,
    new_worker: ReaderNewWorkerFunc,
}

unsafe impl Send for ReaderBox {}
unsafe impl Codable for ReaderBox {}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            new_worker: abi_reader_new_worker,
        }
    }

    pub fn new_worker(&self, ctx: &Context, args: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
        let mut err = SafeString::new();
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
            Err(format_err!("{}", err))
        }
    }
}

extern "C" fn abi_reader_new_worker(
    reader: *mut Box<Reader>,
    ctx: *const Context,
    arg: *const u8,
    arg_len: u64,
    out: *mut WorkerBox,
    err: *mut SafeString,
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
            unsafe { *err = SafeString::from(&format!("{}", e)) };
            0
        }
    }
}

/// Reader worker trait.
pub trait Worker: Send {
    fn read(&mut self) -> Result<Vec<ByteSlice>>;
    fn layer_id(&self) -> Token;
}

type ReaderFunc = extern "C" fn(*mut Box<Worker>, *mut SafeVec<ByteSlice>, *mut SafeString) -> u8;

pub struct WorkerBox {
    worker: *mut Box<Worker>,
    read: ReaderFunc,
    layer_id: extern "C" fn(*const Box<Worker>) -> u32,
    drop: extern "C" fn(*mut Box<Worker>),
}

unsafe impl Send for WorkerBox {}

impl WorkerBox {
    pub fn new(worker: Box<Worker>) -> WorkerBox {
        Self {
            worker: Box::into_raw(Box::new(worker)),
            read: abi_reader_worker_read,
            layer_id: abi_reader_worker_layer_id,
            drop: abi_reader_worker_drop,
        }
    }

    pub fn read(&mut self) -> Result<Vec<ByteSlice>> {
        let mut v = SafeVec::new();
        let mut e = SafeString::new();
        if (self.read)(self.worker, &mut v, &mut e) == 0 {
            Err(format_err!("{}", e))
        } else {
            Ok(v.into_iter().collect())
        }
    }

    pub fn layer_id(&self) -> Token {
        (self.layer_id)(self.worker).into()
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

extern "C" fn abi_reader_worker_layer_id(worker: *const Box<Worker>) -> u32 {
    let worker = unsafe { &*worker };
    worker.layer_id().into()
}

extern "C" fn abi_reader_worker_read(
    worker: *mut Box<Worker>,
    out: *mut SafeVec<ByteSlice>,
    err: *mut SafeString,
) -> u8 {
    let worker = unsafe { &mut *worker };
    match worker.read() {
        Ok(slices) => {
            let mut safe = SafeVec::with_capacity(slices.len() as u64);
            for slice in slices {
                safe.push(slice);
            }
            unsafe { *out = safe };
            1
        }
        Err(e) => {
            unsafe { *err = SafeString::from(&format!("{}", e)) };
            0
        }
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct ReaderData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub reader: CodedData<ReaderBox>,
}

pub struct ReaderBuilder {
    data: ReaderData,
}

impl<T: 'static + Reader> IntoBuilder<ReaderBuilder> for T {
    fn into_builder(self) -> ReaderBuilder {
        ReaderBuilder {
            data: ReaderData {
                id: String::new(),
                filters: Vec::new(),
                reader: CodedData::new(ReaderBox::new(self)),
            },
        }
    }
}

impl ReaderBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn filter<T: Into<FileType>>(mut self, file: T) -> Self {
        self.data.filters.push(file.into());
        self
    }
}

impl Into<ReaderData> for ReaderBuilder {
    fn into(self) -> ReaderData {
        self.data
    }
}

impl Into<Component> for ReaderBuilder {
    fn into(self) -> Component {
        Component::Reader(self.data)
    }
}
