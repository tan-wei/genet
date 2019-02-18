use crate::{
    bytes::Bytes,
    codable::{Codable, CodedData},
    context::Context,
    file::FileType,
    package::IntoBuilder,
    result::Result,
    string::SafeString,
    vec::SafeVec,
};
use failure::format_err;
use serde_derive::{Deserialize, Serialize};
use std::{fmt, mem, ptr, slice, str};
use url::Url;

/// Reader trait.
pub trait Reader: Send {
    fn new_worker(&self, ctx: &Context, url: &Url) -> Result<Box<Worker>>;
}

type ReaderNewWorkerFunc = unsafe extern "C" fn(
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

    pub fn new_worker(&self, ctx: &Context, url: &str) -> Result<WorkerBox> {
        let mut out: WorkerBox = unsafe { mem::uninitialized() };
        let mut err = SafeString::new();
        let result = unsafe {
            (self.new_worker)(
                self.reader,
                ctx,
                url.as_ptr(),
                url.len() as u64,
                &mut out,
                &mut err,
            )
        };
        if result == 1 {
            Ok(out)
        } else {
            mem::forget(out);
            Err(format_err!("{}", err))
        }
    }
}

unsafe extern "C" fn abi_reader_new_worker(
    reader: *mut Box<Reader>,
    ctx: *const Context,
    url: *const u8,
    url_len: u64,
    out: *mut WorkerBox,
    err: *mut SafeString,
) -> u8 {
    let reader = &*reader;
    let ctx = &*ctx;
    let url = str::from_utf8_unchecked(slice::from_raw_parts(url, url_len as usize));
    let url = Url::parse(url)
        .ok()
        .unwrap_or_else(|| Url::parse("null:").unwrap());
    match reader.new_worker(ctx, &url) {
        Ok(worker) => {
            ptr::write(out, WorkerBox::new(worker));
            1
        }
        Err(e) => {
            *err = SafeString::from(&format!("{}", e));
            0
        }
    }
}

/// Reader worker trait.
pub trait Worker: Send {
    fn read(&mut self) -> Result<Vec<Bytes>>;
    fn layer_id(&self) -> &str;
}

type ReaderFunc =
    unsafe extern "C" fn(*mut Box<Worker>, *mut SafeVec<Bytes>, *mut SafeString) -> u8;

pub struct WorkerBox {
    worker: *mut Box<Worker>,
    read: ReaderFunc,
    layer_id: unsafe extern "C" fn(*const Box<Worker>, *mut u64) -> *const u8,
    drop: unsafe extern "C" fn(*mut Box<Worker>),
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

    pub fn read(&mut self) -> Result<Vec<Bytes>> {
        let mut v = SafeVec::new();
        let mut e = SafeString::new();
        if unsafe { (self.read)(self.worker, &mut v, &mut e) } == 0 {
            Err(format_err!("{}", e))
        } else {
            Ok(v.into_iter().collect())
        }
    }

    pub fn layer_id(&self) -> &str {
        let mut len = 0;
        unsafe {
            let data = (self.layer_id)(self.worker, &mut len);
            str::from_utf8_unchecked(slice::from_raw_parts(data, len as usize))
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
        unsafe { (self.drop)(self.worker) };
    }
}

unsafe extern "C" fn abi_reader_worker_drop(worker: *mut Box<Worker>) {
    Box::from_raw(worker);
}

unsafe extern "C" fn abi_reader_worker_layer_id(
    worker: *const Box<Worker>,
    len: *mut u64,
) -> *const u8 {
    let worker = &*worker;
    let id = worker.layer_id();
    *len = id.len() as u64;
    id.as_ptr()
}

unsafe extern "C" fn abi_reader_worker_read(
    worker: *mut Box<Worker>,
    out: *mut SafeVec<Bytes>,
    err: *mut SafeString,
) -> u8 {
    let worker = &mut *worker;
    match worker.read() {
        Ok(slices) => {
            let mut safe = SafeVec::with_capacity(slices.len() as u64);
            for slice in slices {
                safe.push(slice);
            }
            *out = safe;
            1
        }
        Err(e) => {
            *err = SafeString::from(&format!("{}", e));
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

impl<T: 'static + Reader> IntoBuilder<ReaderData> for T {
    fn into_builder(self) -> ReaderData {
        ReaderData {
            id: String::new(),
            filters: Vec::new(),
            reader: CodedData::new(ReaderBox::new(self)),
        }
    }
}

impl ReaderData {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.id = id.into();
        self
    }

    pub fn filter<T: Into<FileType>>(mut self, file: T) -> Self {
        self.filters.push(file.into());
        self
    }
}
