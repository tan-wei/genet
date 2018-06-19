use libc;
use profile::Profile;
use session::{Callback, Event, Session};
use std::error::Error;
use std::ffi::{CStr, CString};
use std::ptr;
use std::str;

#[no_mangle]
pub extern "C" fn genet_session_profile_new() -> *mut Profile {
    Box::into_raw(Box::new(Profile::new()))
}

#[no_mangle]
pub extern "C" fn genet_session_profile_concurrency(profile: *const Profile) -> u32 {
    unsafe { (*profile).concurrency() }
}

#[no_mangle]
pub extern "C" fn genet_session_profile_set_concurrency(profile: *mut Profile, concurrency: u32) {
    unsafe { (*profile).set_concurrency(concurrency) }
}

#[no_mangle]
pub extern "C" fn genet_session_profile_load_library(
    profile: *mut Profile,
    path: *const libc::c_char,
) -> *mut libc::c_char {
    unsafe {
        let path = str::from_utf8_unchecked(CStr::from_ptr(path).to_bytes());
        if let Err(err) = (*profile).load_library(path) {
            CString::new(err.description()).unwrap().into_raw()
        } else {
            ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn genet_session_profile_set_config(
    profile: *mut Profile,
    key: *const libc::c_char,
    value: *const libc::c_char,
) {
    unsafe {
        let key = str::from_utf8_unchecked(CStr::from_ptr(key).to_bytes());
        let value = str::from_utf8_unchecked(CStr::from_ptr(value).to_bytes());
        (*profile).set_config(key, value);
    }
}

#[no_mangle]
pub extern "C" fn genet_session_profile_free(profile: *mut Profile) {
    unsafe {
        if !profile.is_null() {
            Box::from_raw(profile);
        }
    }
}

/*
use libc;
use plugkit::{
    context::Context, frame::{Frame, Raw},
};
use profile::Profile;
use serde_json;
use session::{Callback, Event, Session};
use std::ptr;
use std::slice;

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
pub extern "C" fn genet_session_new(
    profile: *const Profile,
    callback: extern "C" fn(*mut libc::c_void, *mut libc::c_char),
    data: *mut libc::c_void,
) -> *mut Session {
    let holder = DataHolder { callback, data };
    unsafe { Box::into_raw(Box::new(Session::new((*profile).clone(), holder))) }
}

#[no_mangle]
pub extern "C" fn genet_session_frames(
    session: *const Session,
    start: u32,
    end: u32,
    len: *mut libc::size_t,
    dst: *mut *const Frame,
) {
    let frames = unsafe { (*session).frames(start as usize..end as usize) };
    let mut frames = frames
        .into_iter()
        .map(|f| f.as_ref() as *const Frame)
        .collect::<Vec<_>>();
    unsafe {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, *len);
        *len = frames.len();
    }
}

#[no_mangle]
pub extern "C" fn genet_session_filtered_frames(
    session: *const Session,
    id: u32,
    start: u32,
    end: u32,
    len: *mut libc::size_t,
    dst: *mut *const Frame,
) {
    let frames = unsafe { (*session).filtered_frames(id, start as usize..end as usize) };
    let mut frames = frames
        .into_iter()
        .map(|f| f.as_ref() as *const Frame)
        .collect::<Vec<_>>();
    unsafe {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, *len);
        *len = frames.len();
    }
}

#[no_mangle]
pub extern "C" fn genet_session_push_frame(
    session: *mut Session,
    data: *const libc::c_char,
    len: u32,
    link: i32,
) {
    unsafe {
        let data = slice::from_raw_parts(data as *const u8, len as usize);
        let raw = Raw::new(data.into(), len, 0, 0);
        let mut v = Vec::new();
        v.push(raw);
        (*session).push_frames(v, link);
    }
}

#[no_mangle]
pub extern "C" fn genet_session_context(session: *const Session) -> *mut Context {
    unsafe { Box::into_raw(Box::new((*session).context())) }
}

#[no_mangle]
pub extern "C" fn genet_session_len(session: *const Session) -> u32 {
    unsafe { (*session).len() as u32 }
}

#[no_mangle]
pub extern "C" fn genet_session_free(session: *mut Session) {
    unsafe {
        if !session.is_null() {
            Box::from_raw(session);
        }
    }
}
*/
