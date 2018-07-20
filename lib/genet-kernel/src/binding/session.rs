use filter::{Filter, Worker};
use frame::Frame;
use libc;
use profile::Profile;
use serde_json;
use session::{Callback, Event, Session};
use std::{
    cmp,
    error::Error,
    ffi::{CStr, CString},
    ptr, str,
};

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_new() -> *mut Profile {
    Box::into_raw(Box::new(Profile::new()))
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_concurrency(profile: *const Profile) -> u32 {
    (*profile).concurrency()
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_set_concurrency(
    profile: *mut Profile,
    concurrency: u32,
) {
    (*profile).set_concurrency(concurrency)
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_load_library(
    profile: *mut Profile,
    path: *const libc::c_char,
) -> *mut libc::c_char {
    let path = str::from_utf8_unchecked(CStr::from_ptr(path).to_bytes());
    if let Err(err) = (*profile).load_library(path) {
        CString::new(err.description()).unwrap().into_raw()
    } else {
        ptr::null_mut()
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_set_config(
    profile: *mut Profile,
    key: *const libc::c_char,
    value: *const libc::c_char,
) {
    let key = str::from_utf8_unchecked(CStr::from_ptr(key).to_bytes());
    let value = str::from_utf8_unchecked(CStr::from_ptr(value).to_bytes());
    (*profile).set_config(key, value);
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_free(profile: *mut Profile) {
    if !profile.is_null() {
        Box::from_raw(profile);
    }
}

struct DataHolder {
    callback: extern "C" fn(*mut libc::c_void, *mut libc::c_char),
    data: *mut libc::c_void,
}

unsafe impl Sync for DataHolder {}
unsafe impl Send for DataHolder {}

impl Callback for DataHolder {
    fn on_event(&self, event: Event) {
        let json = CString::new(serde_json::to_string(&event).unwrap())
            .unwrap()
            .into_raw();
        (self.callback)(self.data, json);
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_new(
    profile: *const Profile,
    callback: extern "C" fn(*mut libc::c_void, *mut libc::c_char),
    data: *mut libc::c_void,
) -> *mut Session {
    let holder = DataHolder { callback, data };
    Box::into_raw(Box::new(Session::new((*profile).clone(), holder)))
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_frames(
    session: *const Session,
    start: u32,
    end: u32,
    len: *mut u32,
    dst: *mut *const Frame,
) {
    let frames = (*session).frames(start as usize..end as usize);
    let size = cmp::min(*len as usize, frames.len());
    if size > 0 {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, size);
    }
    *len = size as u32;
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_filtered_frames(
    session: *const Session,
    id: u32,
    start: u32,
    end: u32,
    len: *mut u32,
    dst: *mut u32,
) {
    let frames = (*session).filtered_frames(id, start as usize..end as usize);
    let size = cmp::min(*len as usize, frames.len());
    if size > 0 {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, size);
    }
    *len = size as u32;
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_create_reader(
    session: *mut Session,
    id: *const libc::c_char,
    arg: *const libc::c_char,
) -> u32 {
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    let arg = str::from_utf8_unchecked(CStr::from_ptr(arg).to_bytes());
    (*session).create_reader(id, arg)
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_create_writer(
    session: *mut Session,
    id: *const libc::c_char,
    arg: *const libc::c_char,
    filter: *mut FilterBase,
) -> u32 {
    let filter = if filter.is_null() {
        None
    } else {
        let b: Box<Filter> = Box::new(FFIFilter {
            base: FilterBaseHolder(filter),
        });
        Some(b)
    };
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    let arg = str::from_utf8_unchecked(CStr::from_ptr(arg).to_bytes());
    (*session).create_writer(id, arg, filter)
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_close_reader(session: *mut Session, handle: u32) {
    (*session).close_reader(handle);
}
#[no_mangle]
pub unsafe extern "C" fn genet_session_set_filter(
    session: *mut Session,
    id: u32,
    filter: *mut FilterBase,
) {
    let filter = if filter.is_null() {
        None
    } else {
        let b: Box<Filter> = Box::new(FFIFilter {
            base: FilterBaseHolder(filter),
        });
        Some(b)
    };
    (*session).set_filter(id, filter);
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_len(session: *const Session) -> u32 {
    (*session).len() as u32
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_free(session: *mut Session) {
    if !session.is_null() {
        Box::from_raw(session);
    }
}

#[repr(C)]
pub struct FilterBase {
    new_worker: extern "C" fn(*mut FilterBase) -> *mut FilterWorkerBase,
    destroy: extern "C" fn(*mut FilterBase),
}

#[derive(Debug)]
pub struct FilterBaseHolder(*mut FilterBase);

unsafe impl Send for FilterBaseHolder {}

impl Drop for FilterBaseHolder {
    fn drop(&mut self) {
        unsafe { ((*self.0).destroy)(self.0) };
    }
}

#[repr(C)]
pub struct FilterWorkerBase {
    test: extern "C" fn(*mut FilterWorkerBase, *const Frame) -> u8,
    destroy: extern "C" fn(*mut FilterWorkerBase),
}

pub struct FilterWorkerBaseHolder(*mut FilterWorkerBase);

unsafe impl Send for FilterWorkerBaseHolder {}

impl Drop for FilterWorkerBaseHolder {
    fn drop(&mut self) {
        unsafe { ((*self.0).destroy)(self.0) };
    }
}

#[derive(Debug)]
pub struct FFIFilter {
    base: FilterBaseHolder,
}

impl Filter for FFIFilter {
    fn new_worker(&self) -> Box<Worker> {
        let filter: *mut FilterBase = self.base.0;
        unsafe {
            Box::new(FFIFilterWorker {
                base: FilterWorkerBaseHolder(((*filter).new_worker)(filter)),
            })
        }
    }
}

pub struct FFIFilterWorker {
    base: FilterWorkerBaseHolder,
}

impl Worker for FFIFilterWorker {
    fn test(&self, frame: &Frame) -> bool {
        let worker: *mut FilterWorkerBase = self.base.0;
        unsafe { ((*worker).test)(worker, frame) != 0 }
    }
}
